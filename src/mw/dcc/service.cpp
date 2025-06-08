// Copyright (C) 2025 Vincent Hamp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "service.hpp"
#include <ArduinoJson.h>
#include <esp_task.h>
#include <static_math/static_math.h>
#include <dcc/dcc.hpp>
#include <ranges>
#include "drv/led/bug.hpp"
#include "log.h"
#include "mem/nvs/locos.hpp"
#include "mem/nvs/settings.hpp"
#include "utility.hpp"

namespace dcc {

using namespace std::literals;

/// \todo document
Service::Service() {
  for (mem::nvs::Locos nvs; auto const& entry_info : nvs) {
    auto const addr{nvs.key2address(entry_info.key)};
    dynamic_cast<NvLocoBase&>(_locos[addr]) = nvs.get(entry_info.key);
  }
  task.create(ztl::make_trampoline(this, &Service::taskFunction));
}

/// \todo document
Service::~Service() { task.destroy(); }

/// \todo document
void Service::z21(std::shared_ptr<z21::server::intf::System> z21_system_service,
                  std::shared_ptr<z21::server::intf::Dcc> z21_dcc_service) {
  _z21_system_service = z21_system_service;
  _z21_dcc_service = z21_dcc_service;
}

/// \todo document
/// \todo filters?
intf::http::Response Service::locosGetRequest(intf::http::Request const& req) {
  // Singleton
  if (auto const addr{uri2address(req.uri).value_or(0u)}) {
    auto const it{_locos.find(addr)};
    if (it == cend(_locos))
      return std::unexpected<std::string>{"404 Not Found"};
    auto doc{it->second.toJsonDocument()};
    doc["address"] = addr;
    std::string json;
    json.reserve(1024uz);
    serializeJson(doc, json);
    return json;
  }
  // Collection
  else {
    JsonDocument doc;
    auto array{doc.to<JsonArray>()}; // Explicitly convert to array
    for (auto const& [addr, loco] : _locos) {
      auto loco_doc{loco.toJsonDocument()};
      loco_doc["address"] = addr;
      array.add(loco_doc);
    }
    std::string json;
    json.reserve((size(_locos) + 1uz) * 1024uz);
    serializeJson(doc, json);
    return json;
  }
}

/// \todo document
/// \todo filters?
intf::http::Response
Service::locosDeleteRequest(intf::http::Request const& req) {
  auto const addr{uri2address(req.uri).value_or(0u)};

  // Singleton
  if (std::lock_guard lock{_internal_mutex}; addr) {
    // Erase (doesn't matter if it exists or not)
    _locos.erase(addr);
    mem::nvs::Locos nvs;
    nvs.erase(addr);
  }
  // Collection
  else if (req.uri == "/dcc/locos/"sv) {
    // Erase all
    _locos.clear();
    mem::nvs::Locos nvs;
    nvs.eraseAll();
  }

  return {};
}

/// \todo document
intf::http::Response Service::locosPutRequest(intf::http::Request const& req) {
  // Validate body
  if (!validate_json(req.body))
    return std::unexpected<std::string>{"415 Unsupported Media Type"};

  // Address not found or other characters appended to it
  // We currently only support singleton
  auto addr{uri2address(req.uri).value_or(0u)};
  if (!addr) return std::unexpected<std::string>{"417 Expectation Failed"};

  // Deserialize
  JsonDocument doc;
  if (auto const err{deserializeJson(doc, req.body)}) {
    LOGE("Deserialization failed %s", err.c_str());
    return std::unexpected<std::string>{"500 Internal Server Error"};
  }

  auto it{_locos.find(addr)};
  mem::nvs::Locos nvs;

  // Address not found
  if (std::lock_guard lock{_internal_mutex}; it == cend(_locos)) {
    // Address in URI does not match body
    if (JsonVariantConst v{doc["address"]}; v.as<Address::value_type>() != addr)
      return std::unexpected<std::string>{"417 Expectation Failed"};
    // Insert new loco
    else if (auto const ret{_locos.insert({addr, Loco{doc}})}; ret.second)
      it = ret.first; // Update iterator
    // Insertion failed
    else return std::unexpected<std::string>{"500 Internal Server Error"};
  }
  // Address found, but changing
  else if (JsonVariantConst v{doc["address"]};
           v.as<Address::value_type>() != addr) {
    auto node{_locos.extract(addr)};
    node.key() = v.as<Address::value_type>();
    // Re-insert loco with new address
    if (auto const ret{_locos.insert(move(node))}; ret.inserted) {
      it = ret.position;                  // Update iterator
      nvs.erase(addr);                    // Erase old address
      addr = v.as<Address::value_type>(); // Update address
    }
    // Insertion failed
    else
      return std::unexpected<std::string>{"500 Internal Server Error"};
  }
  // Address found, just update loco
  else
    it->second.fromJsonDocument(doc); // Update iterator

  nvs.set(addr, it->second);

  return {};
}

/// \todo document
void Service::taskFunction(void*) {
  for (;;) switch (state.load()) {
      case State::DCCOperations:
        resume();
        operationsLoop();
        suspend();
        break;
      case State::DCCService:
        resume();
        serviceLoop();
        suspend();
        break;
      default: LOGI_TASK_SUSPEND(); break;
    }
}

/// \todo document
void Service::operationsLoop() {
  while (state.load() == State::DCCOperations) {
    operationsDcc();
    operationsBiDi();
    vTaskDelay(pdMS_TO_TICKS(task.timeout));

    //
    if (!empty(_cv_request_deque)) return serviceLoop();
  }

  // wait for task to get suspended
  while (eTaskGetState(drv::out::track::dcc::task.handle) != eSuspended)
    vTaskDelay(pdMS_TO_TICKS(task.timeout));
}

/// Currently fills message buffer between 25 and 50%
void Service::operationsDcc() {
  // Less than 50% space available
  if (xMessageBufferSpacesAvailable(drv::out::tx_message_buffer.back_handle) <
      drv::out::tx_message_buffer.size * 0.5)
    return;

  // So, we iterate over each loco and check it's priority
  // if it's
  // if (!(_priority_count % priority))
  // then we push a bunch of commands... and increment its priority
  // The maximum priority must be some remainder of 256 though (e.g. 32?),
  // otherwise it's pointless.
  // Can we get in trouble if we address the same loco X times in a row? Are
  // there are circumstances where this is not ok?

  // If all locos are up to maximum priority there must be some kind of "reset"?
  if (_priority_count == Loco::max_priority) {
    _priority_count = Loco::min_priority;
    for (auto& [addr, loco] : _locos) loco.priority = Loco::min_priority;
  }

  //
  for (;;) {
    //
    if (empty(_locos)) {
      sendToBack(make_idle_packet());

      if (xMessageBufferSpacesAvailable(
            drv::out::tx_message_buffer.back_handle) <
          drv::out::tx_message_buffer.size * 0.25)
        return;
    }
    //
    else {
      for (auto& [addr, loco] : _locos) {
        //
        if (_priority_count % loco.priority) continue;

        // Speed and direction
        switch (loco.speed_steps) {
          case z21::LocoInfo::DCC14:
            sendToBack(make_speed_and_direction_packet(
              basicOrExtendedLocoAddress(addr),
              (loco.rvvvvvvv & 0x80u) >> 2u | // R
                (loco.f31_0 & 0x01u) << 4u |  // F0
                (loco.rvvvvvvv & 0x0Fu)));    // GGGG
            break;
          case z21::LocoInfo::DCC28:
            sendToBack(make_speed_and_direction_packet(
              basicOrExtendedLocoAddress(addr),
              (loco.rvvvvvvv & 0x80u) >> 2u  // R
                | (loco.rvvvvvvv & 0x1Fu))); // G-GGGG
            break;
          case z21::LocoInfo::DCC128:
            sendToBack(make_advanced_operations_speed_packet(
              basicOrExtendedLocoAddress(addr), loco.rvvvvvvv));
            break;
        }

        // Lower functions
        sendToBack(make_function_group_f4_f0_packet(
          basicOrExtendedLocoAddress(addr), loco.f31_0 & 0x1Fu));
        sendToBack(make_function_group_f8_f5_packet(
          basicOrExtendedLocoAddress(addr), loco.f31_0 >> 5u & 0xFu));
        sendToBack(make_function_group_f12_f9_packet(
          basicOrExtendedLocoAddress(addr), loco.f31_0 >> 9u & 0xFu));

        // Higher functions
        if (_dcc_loco_flags & z21::MmDccSettings::Flags::RepeatHfx) {
          sendToBack(make_feature_expansion_f20_f13_packet(
            basicOrExtendedLocoAddress(addr), loco.f31_0 >> 13u));
          sendToBack(make_feature_expansion_f28_f21_packet(
            basicOrExtendedLocoAddress(addr), loco.f31_0 >> 21u));
        }

        loco.priority = std::clamp<decltype(loco.priority)>(
          loco.priority + 1u, Loco::min_priority, Loco::max_priority);

        if (xMessageBufferSpacesAvailable(
              drv::out::tx_message_buffer.back_handle) <
            drv::out::tx_message_buffer.size * 0.25)
          return;
      }

      _priority_count = std::clamp<decltype(_priority_count)>(
        _priority_count + 1u, Loco::min_priority, Loco::max_priority);
    }
  }
}

/// \todo document
void Service::operationsBiDi() {
  drv::out::track::RxQueue::value_type item;

  while (xQueueReceive(drv::out::track::rx_queue.handle, &item, 0u)) {
    // Currently only care for loco addresses
    /// \todo remove that once we care for other addresses
    auto const addr{decode_address(item.packet)};
    if (addr.type != Address::BasicLoco && addr.type != Address::ExtendedLoco)
      continue;

    // Channel 1 (which I don't care about currently)
    std::span const ch1{cbegin(item.datagram), cbegin(item.datagram) + 2};

    // Channel 2
    std::span const ch2{cbegin(item.datagram) + 2, cend(item.datagram)};
    // No data
    if (std::ranges::all_of(ch2, [](uint8_t b) { return !b; })) continue;
    // Invalid data
    else if (std::ranges::any_of(ch2, [](uint8_t b) {
               return b && std::popcount(b) != CHAR_BIT / 2;
             }))
      continue;
    // ACK
    else if (std::ranges::any_of(ch2, [](uint8_t b) {
               return b == dcc::bidi::acks[0uz] || b == dcc::bidi::acks[1uz];
             }))
      ; /// \todo do something with ACK information... (e.g. lower priority)

    // Make data
    auto data{bidi::make_data(bidi::decode_datagram(item.datagram))};

    // Remove channel 1 address part (bits 48-36)
    data &= 0xF'FFFF'FFFFull;
    if (!data) continue;

    for (auto i{static_cast<int32_t>(bidi::Bits::_36)}; i > 0;) {
      assert(i > 4);
      switch (
        auto const id{static_cast<uint8_t>((data >> (i - 4)) & 0b1111u)}) {
        // app:pom
        case 0u:
          if (i == static_cast<int32_t>(bidi::Bits::_36) &&
              !empty(_cv_pom_request_deque)) {
            if (decode_instruction(item.packet) == Instruction::CvLong) {
              auto const off{addr.type == Address::ExtendedLoco};
              auto const cv_addr{(item.packet[1uz + off] & 0b11u) << 8u |
                                 item.packet[2uz + off]};
              if (auto const& req{_cv_pom_request_deque.front()};
                  req.addr == addr && req.cv_addr == cv_addr) {
                cvAck(cv_addr, static_cast<uint8_t>(data >> 24u));
                _cv_pom_request_deque.pop_front();
              }
            }
          }
          i -= static_cast<int32_t>(bidi::Bits::_12);
          break;

        // app:adr_high
        case 1u: i = 0; break;

        // app:adr_low
        case 2u: i = 0; break;

        // app:ext
        case 3u: i = 0; break;

        // app:info
        case 4u: i = 0; break;

        // app:dyn
        case 7u: {
          if (auto const it{_locos.find(addr)}; it != cend(_locos)) {
            auto const x{static_cast<uint8_t>((data >> (i - 18)) & 0b11'1111u)};
            auto const d{static_cast<uint8_t>(data >> (i - 12))};
            it->second.bidi.loco_address = addr;
            auto const bidi_before{it->second.bidi};
            switch (x) {
              // Speed (<=255)
              case 0u:
                it->second.bidi.options =
                  static_cast<z21::RailComData::Options>(
                    (it->second.bidi.options &
                     ~(z21::RailComData::Options::Speed2 |
                       z21::RailComData::Options::Speed1)) |
                    z21::RailComData::Options::Speed1);
                it->second.bidi.speed = d;
                break;
              // Speed (>255)
              case 1u:
                it->second.bidi.options =
                  static_cast<z21::RailComData::Options>(
                    (it->second.bidi.options &
                     ~(z21::RailComData::Options::Speed2 |
                       z21::RailComData::Options::Speed1)) |
                    z21::RailComData::Options::Speed2);
                it->second.bidi.speed = d;
                break;
              // QoS
              case 7u:
                it->second.bidi.options =
                  static_cast<z21::RailComData::Options>(
                    it->second.bidi.options | z21::RailComData::Options::QoS);
                it->second.bidi.qos = d;
                break;
            }
            if (it->second.bidi != bidi_before) broadcastRailComData(addr);
          }
          i -= static_cast<int32_t>(bidi::Bits::_18);
          break;
        }

        // app:xpom
        case 8u: i = 0; break;

        // app:xpom
        case 9u: i = 0; break;

        // app:xpom
        case 10u: i = 0; break;

        // app:xpom
        case 11u: i = 0; break;

        // app:CV-auto
        case 12u: i = 0; break;

        // app:block
        case 13u: i = 0; break;

        // app:zeit
        case 14u: i = 0; break;

        // Error
        default: i = 0; break;
      }
    }
  }

  // Handle POM timeouts
  if (!empty(_cv_pom_request_deque) &&
      xTaskGetTickCount() > _cv_pom_request_deque.front().then) {
    _cv_pom_request_deque.pop_front();
    cvNack();
  }
}

/// \todo document
void Service::serviceLoop() {
  drv::led::Bug const led_bug{};

  if (empty(_cv_request_deque)) return;

  /// \todo oh god please make this safer...
  /// it changes from opmode to serv...
  if (auto expected{State::DCCOperations};
      state.compare_exchange_strong(expected, State::Suspend)) {

    // wait for task to get suspended
    while (eTaskGetState(drv::out::track::dcc::task.handle) != eSuspended)
      vTaskDelay(pdMS_TO_TICKS(task.timeout));

    // switch to serv mode
    expected = State::Suspended;
    if (!state.compare_exchange_strong(expected, State::DCCService))
      assert(false);

    // then resume
    LOGI_TASK_RESUME(drv::out::track::dcc::task);
  }

  auto const& req{_cv_request_deque.front()};
  auto const cv_addr{req.cv_addr};
  auto const byte{req.byte ? serviceWrite(cv_addr, *req.byte)
                           : serviceRead(cv_addr)};
  _cv_request_deque.pop_front();

  // wait for task to get suspended
  while (eTaskGetState(drv::out::track::dcc::task.handle) != eSuspended)
    vTaskDelay(pdMS_TO_TICKS(task.timeout));

  // send reply
  if (byte) cvAck(cv_addr, *byte);
  else cvNack();
}

/// \todo document
std::optional<uint8_t> Service::serviceRead(uint16_t cv_addr) {
  std::optional<uint8_t> byte{};

  // Nothing
  if (_programming_type == z21::CommonSettings::ProgrammingType::Nothing)
    return std::nullopt;

  // Byte verify only
  if (_programming_type == z21::CommonSettings::ProgrammingType::ByteOnly) {
    for (auto i{0u}; i < std::numeric_limits<uint8_t>::max(); ++i) {
      sendToFront(make_cv_access_long_verify_service_packet(cv_addr, i),
                  _program_packet_count);
      if (serviceReceiveBit() == true) return i;
    }
  }

  // Bit verify
  if (_programming_type & z21::CommonSettings::ProgrammingType::BitOnly) {
    for (uint8_t i{0u}; i < CHAR_BIT; ++i)
      sendToFront(
        make_cv_access_long_verify_service_packet(cv_addr, _bit_verify_to_1, i),
        _program_packet_count);
    byte = serviceReceiveByte();

    // Only
    if (_programming_type == z21::CommonSettings::ProgrammingType::BitOnly)
      return byte;
  }

  // Bit and byte verify
  if (_programming_type == z21::CommonSettings::ProgrammingType::Both && byte) {
    sendToFront(make_cv_access_long_verify_service_packet(cv_addr, *byte),
                _program_packet_count);
    if (serviceReceiveBit() == true) return byte;
  }

  return std::nullopt;
}

/// \todo document
std::optional<uint8_t> Service::serviceWrite(uint16_t cv_addr, uint8_t byte) {
  sendToFront(make_cv_access_long_write_service_packet(cv_addr, byte),
              _program_packet_count);

  if (serviceReceiveBit() == true) return byte;

  return std::nullopt;
}

/// \todo document
/// Depending on the DCC settings we might need to wait a long ass time...
std::optional<bool> Service::serviceReceiveBit() {
  bool bit;
  if (xMessageBufferReceive(drv::out::rx_message_buffer.handle,
                            &bit,
                            sizeof(bit),
                            pdMS_TO_TICKS(std::numeric_limits<uint8_t>::max() *
                                          10u)) == sizeof(bit))
    return bit;
  else return std::nullopt;
}

/// \todo document
std::optional<uint8_t> Service::serviceReceiveByte() {
  uint8_t byte{};
  for (auto i{0uz}; i < CHAR_BIT; ++i)
    if (auto const bit{serviceReceiveBit()})
      byte |= static_cast<uint8_t>((*bit == _bit_verify_to_1) << i);
    else return std::nullopt;
  return byte;
}

/// \todo document
void Service::sendToFront(Packet const& packet, size_t n) {
  /*
  This is actually WAY more involved, we need to copy the entire back_handle
  message buffer to some temporary, search it for equal packets (same address,
  same instruction) and then copy all the filtered messages back...
  */
  for (auto i{0uz}; i < n; ++i)
    while (!xMessageBufferSend(drv::out::tx_message_buffer.front_handle,
                               data(packet),
                               size(packet),
                               0u));
}

/// \todo document
void Service::sendToBack(Packet const& packet, size_t n) {
  for (auto i{0uz}; i < n; ++i)
    while (!xMessageBufferSend(
      drv::out::tx_message_buffer.back_handle, data(packet), size(packet), 0u));
}

/// \todo document
z21::LocoInfo Service::locoInfo(uint16_t loco_addr) {
  assert(loco_addr);

  std::lock_guard lock{_internal_mutex};
  auto& loco{_locos[loco_addr]};

  //
  if (empty(loco.name)) loco.name = std::to_string(loco_addr);
  mem::nvs::Locos nvs;
  nvs.set(loco_addr, loco);

  return loco;
}

/// \todo document
void Service::locoDrive(uint16_t loco_addr,
                        z21::LocoInfo::SpeedSteps speed_steps,
                        uint8_t rvvvvvvv) {
  //
  if (!loco_addr) switch (speed_steps) {
      case z21::LocoInfo::DCC14:
        return sendToFront(
          make_speed_and_direction_packet(basicOrExtendedLocoAddress(loco_addr),
                                          (rvvvvvvv & 0x80u) >> 2u | // R
                                            (rvvvvvvv & 0x0Fu)),     // GGGG
          _program_packet_count);
      case z21::LocoInfo::DCC28:
        return sendToFront(
          make_speed_and_direction_packet(basicOrExtendedLocoAddress(loco_addr),
                                          (rvvvvvvv & 0x80u) >> 2u // R
                                            | (rvvvvvvv & 0x1Fu)), // G-GGGG
          _program_packet_count);
      case z21::LocoInfo::DCC128:
        return sendToFront(make_advanced_operations_speed_packet(
                             basicOrExtendedLocoAddress(loco_addr), rvvvvvvv),
                           _program_packet_count);
      default: return;
    }
  //
  else {
    std::lock_guard lock{_internal_mutex};
    auto& loco{_locos[loco_addr]};

    //
    if (loco.speed_steps == speed_steps && loco.rvvvvvvv == rvvvvvvv) return;
    loco.speed_steps = speed_steps;
    loco.rvvvvvvv = rvvvvvvv;

    //
    if (empty(loco.name)) loco.name = std::to_string(loco_addr);
    mem::nvs::Locos nvs;
    nvs.set(loco_addr, loco);
  }

  //
  broadcastLocoInfo(loco_addr);
}

/// \todo document
void Service::locoFunction(uint16_t loco_addr, uint32_t mask, uint32_t state) {
  //
  if (!loco_addr) {
    /// \todo are broadcast functions a thing?
    return;
  }
  //
  else {
    std::lock_guard lock{_internal_mutex};
    auto& loco{_locos[loco_addr]};

    //
    state = (~mask & loco.f31_0) | (mask & state);
    if (loco.f31_0 == state) return;
    loco.f31_0 = state;

    // Higher functions don't get repeated, send them now
    if (mask >= (1u << 13u) &&
        !(_dcc_loco_flags & z21::MmDccSettings::Flags::RepeatHfx)) {
      if (mask & (0xFFu << 13u))
        sendToBack(make_feature_expansion_f20_f13_packet(
          basicOrExtendedLocoAddress(loco_addr), loco.f31_0 >> 13u));
      if (mask & (0xFFu << 21u))
        sendToBack(make_feature_expansion_f28_f21_packet(
          basicOrExtendedLocoAddress(loco_addr), loco.f31_0 >> 21u));
    }

    //
    if (empty(loco.name)) loco.name = std::to_string(loco_addr);
    mem::nvs::Locos nvs;
    nvs.set(loco_addr, loco);
  }

  //
  broadcastLocoInfo(loco_addr);
}

/// \todo document
z21::LocoInfo::Mode Service::locoMode(uint16_t loco_addr) {
  assert(loco_addr);

  std::lock_guard lock{_internal_mutex};
  auto& loco{_locos[loco_addr]};

  //
  if (empty(loco.name)) loco.name = std::to_string(loco_addr);
  mem::nvs::Locos nvs;
  nvs.set(loco_addr, loco);

  return loco.mode;
}

/// \todo document
void Service::locoMode(uint16_t, z21::LocoInfo::Mode mode) {
  if (mode == z21::LocoInfo::MM) LOGW("MM not supported");
}

/// \todo document
void Service::broadcastLocoInfo(uint16_t addr) {
  _z21_dcc_service->broadcastLocoInfo(addr);
}

/// \todo document
z21::TurnoutInfo Service::turnoutInfo(uint16_t accy_addr) {
  LOGW("TODO IMPLEMENTED");
  return {};
}

/// \todo document
z21::AccessoryInfo Service::accessoryInfo(uint16_t accy_addr) {
  LOGW("TODO IMPLEMENTED");
  return {};
}

/// \todo document
void Service::turnout(uint16_t accy_addr, bool p, bool a, bool q) {
  LOGW("TODO IMPLEMENTED");
}

/// \todo document
void Service::accessory(uint16_t accy_addr, uint8_t dddddddd) {
  LOGW("TODO IMPLEMENTED");
}

/// \todo document
z21::TurnoutInfo::Mode Service::turnoutMode(uint16_t accy_addr) {
  LOGW("TODO IMPLEMENTED");
  return {};
}

/// \todo document
void Service::turnoutMode(uint16_t accy_addr, z21::TurnoutInfo::Mode mode) {
  LOGW("TODO IMPLEMENTED");
}

/// \todo document
void Service::broadcastTurnoutInfo(uint16_t accy_addr) {
  LOGW("TODO IMPLEMENTED");
}

/// \todo document
void Service::broadcastExtAccessoryInfo(uint16_t accy_addr) {
  LOGW("TODO IMPLEMENTED");
}

/// \todo document
bool Service::cvRead(uint16_t cv_addr) {
  if (full(_cv_request_deque)) return false;
  _cv_request_deque.push_back({.cv_addr = cv_addr});
  return true;
}

/// \todo document
bool Service::cvWrite(uint16_t cv_addr, uint8_t byte) {
  if (full(_cv_request_deque)) return false;
  _cv_request_deque.push_back({.cv_addr = cv_addr, .byte = byte});
  return true;
}

/// \todo document
void Service::cvPomRead(uint16_t loco_addr, uint16_t cv_addr) {
  if (full(_cv_pom_request_deque)) return cvNack();

  sendToFront(make_cv_access_long_verify_packet(
                basicOrExtendedLocoAddress(loco_addr), cv_addr),
              _program_packet_count);

  _cv_pom_request_deque.push_back(
    {.then = xTaskGetTickCount() + pdMS_TO_TICKS(500u), // See RCN-217
     .addr = loco_addr,
     .cv_addr = cv_addr});

  /// \todo reset loco prio here

  // Mandatory delay
  vTaskDelay(
    pdMS_TO_TICKS((_program_packet_count + 1u) * 10u)); // ~10ms per packet
}

/// \todo document
void Service::cvPomWrite(uint16_t loco_addr, uint16_t cv_addr, uint8_t byte) {
  sendToFront(make_cv_access_long_write_packet(
                basicOrExtendedLocoAddress(loco_addr), cv_addr, byte),
              _program_packet_count);

  // Mandatory delay
  vTaskDelay(
    pdMS_TO_TICKS((_program_packet_count + 1u) * 10u)); // ~10ms per packet
}

/// \todo document
void Service::cvPomAccessoryRead(uint16_t accy_addr, uint16_t cv_addr) {
  assert(false);
}

/// \todo document
void Service::cvPomAccessoryWrite(uint16_t accy_addr,
                                  uint16_t cv_addr,
                                  uint8_t byte) {
  assert(false);
}

/// \todo document
void Service::cvNackShortCircuit() { _z21_dcc_service->cvNackShortCircuit(); }

/// \todo document
void Service::cvNack() { _z21_dcc_service->cvNack(); }

/// \todo document
void Service::cvAck(uint16_t cv_addr, uint8_t byte) {
  _z21_dcc_service->cvAck(cv_addr, byte);
}

/// \todo document
z21::RailComData Service::railComData(uint16_t loco_addr) {
  auto const it{_locos.find(loco_addr)};
  return it != cend(_locos) ? it->second.bidi
                            : z21::RailComData{.loco_address = loco_addr};
}

/// \todo document
void Service::broadcastRailComData(uint16_t loco_addr) {
  _z21_dcc_service->broadcastRailComData(loco_addr);
}

/// \todo document
void Service::resume() {
  // Update settings
  mem::nvs::Settings nvs;
  _dcc_loco_flags = nvs.getDccLocoFlags();
  _programming_type = nvs.getDccProgrammingType();
  _program_packet_count = nvs.getDccProgramPacketCount();
  _bit_verify_to_1 = nvs.getDccBitVerifyTo1();

  // Preload
  auto const packet{state.load() == State::DCCOperations ? make_idle_packet()
                                                         : make_reset_packet()};
  while (
    xMessageBufferSpacesAvailable(drv::out::tx_message_buffer.back_handle) >
    drv::out::tx_message_buffer.size * 0.5)
    sendToBack(packet);

  // Resume out::track::dcc task
  LOGI_TASK_RESUME(drv::out::track::dcc::task);
}

/// \todo document
void Service::suspend() {
  _priority_count = 0uz;
  _cv_request_deque.clear();
  _cv_pom_request_deque.clear();
  _z21_system_service->broadcastTrackPowerOff();
}

/// \todo document
Address Service::basicOrExtendedLocoAddress(Address::value_type addr) const {
  return {.value = addr,
          .type =
            addr <= (_dcc_loco_flags & z21::MmDccSettings::Flags::DccShort127
                       ? 127u
                       : 99u)
              ? Address::BasicLoco
              : Address::ExtendedLoco};
}

} // namespace dcc
