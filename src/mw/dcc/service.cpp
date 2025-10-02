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
#include "mem/nvs/accessories.hpp"
#include "mem/nvs/locos.hpp"
#include "mem/nvs/settings.hpp"
#include "mem/nvs/turnouts.hpp"
#include "system_state.hpp"
#include "utility.hpp"

namespace mw::dcc {

using namespace std::literals;

/// \todo document
Service::Service() {
  for (mem::nvs::Locos nvs; auto const& entry_info : nvs) {
    auto const addr{mem::nvs::key2address(entry_info.key)};
    dynamic_cast<NvLocoBase&>(_locos[addr]) = nvs.get(entry_info.key);
  }

  for (mem::nvs::Turnouts nvs; auto const& entry_info : nvs) {
    auto const addr{mem::nvs::key2address(entry_info.key)};
    dynamic_cast<NvTurnoutBase&>(_turnouts[addr]) = nvs.get(entry_info.key);
  }

  task.function = ztl::make_trampoline(this, &Service::taskFunction);
}

/// \todo document
void Service::z21(std::shared_ptr<z21::server::intf::System> z21_system_service,
                  std::shared_ptr<z21::server::intf::Dcc> z21_dcc_service) {
  _z21_system_service = z21_system_service;
  _z21_dcc_service = z21_dcc_service;
}

/// \todo document
intf::http::Response Service::getRequest(intf::http::Request const& req) {
  SystemState const system_state{
    static_cast<SystemState&>(_z21_system_service->systemState())};
  auto doc{system_state.toJsonDocument()};
  std::string json;
  json.reserve(1024uz);
  serializeJson(doc, json);
  return json;
}

/// \todo document
intf::http::Response Service::postRequest(intf::http::Request const& req) {
  // Validate body
  if (!validate_json(req.body))
    return std::unexpected<std::string>{"415 Unsupported Media Type"};

  // Deserialize
  JsonDocument doc;
  if (auto const err{deserializeJson(doc, req.body)}) {
    LOGE("Deserialization failed %s", err.c_str());
    return std::unexpected<std::string>{"500 Internal Server Error"};
  }

  // LAN_X_SET_TRACK_POWER_OFF / LAN_X_SET_TRACK_POWER_ON
  if (JsonVariantConst v{doc["central_state"]}; v.is<uint8_t>()) {
    if (auto const is_on{!std::to_underlying(
          _z21_system_service->systemState().central_state &
          z21::CentralState::TrackVoltageOff)},
        should_be_on{!(v.as<uint8_t>() &
                       std::to_underlying(z21::CentralState::TrackVoltageOff))};
        !is_on && should_be_on) {
      _z21_system_service->trackPower(true);
      _z21_system_service->broadcastTrackPowerOn();
    } else if (is_on && !should_be_on) {
      _z21_system_service->trackPower(false);
      _z21_system_service->broadcastTrackPowerOff();
    }
  }

  return {};
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
intf::http::Response
Service::turnoutsGetRequest(intf::http::Request const& req) {
  // Singleton
  if (auto const addr{uri2address(req.uri).value_or(0u)}) {
    auto const it{_turnouts.find(addr)};
    if (it == cend(_turnouts))
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
    for (auto const& [addr, turnout] : _turnouts) {
      auto turnout_doc{turnout.toJsonDocument()};
      turnout_doc["address"] = addr;
      array.add(turnout_doc);
    }
    std::string json;
    json.reserve((size(_turnouts) + 1uz) * 1024uz);
    serializeJson(doc, json);
    return json;
  }
}

/// \todo document
intf::http::Response
Service::turnoutsDeleteRequest(intf::http::Request const& req) {
  auto const addr{uri2address(req.uri).value_or(0u)};

  // Singleton
  if (std::lock_guard lock{_internal_mutex}; addr) {
    // Erase (doesn't matter if it exists or not)
    _turnouts.erase(addr);
    mem::nvs::Turnouts nvs;
    nvs.erase(addr);
  }
  // Collection
  else if (req.uri == "/dcc/turnouts/"sv) {
    // Erase all
    _turnouts.clear();
    mem::nvs::Turnouts nvs;
    nvs.eraseAll();
  }

  return {};
}

/// \todo document
intf::http::Response
Service::turnoutsPutRequest(intf::http::Request const& req) {
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

  auto it{_turnouts.find(addr)};
  mem::nvs::Turnouts nvs;

  // Address not found
  if (std::lock_guard lock{_internal_mutex}; it == cend(_turnouts)) {
    // Address in URI does not match body
    if (JsonVariantConst v{doc["address"]}; v.as<Address::value_type>() != addr)
      return std::unexpected<std::string>{"417 Expectation Failed"};
    // Insert new turnout
    else if (auto const ret{_turnouts.insert({addr, Turnout{doc}})}; ret.second)
      it = ret.first; // Update iterator
    // Insertion failed
    else return std::unexpected<std::string>{"500 Internal Server Error"};
  }
  // Address found, but changing
  else if (JsonVariantConst v{doc["address"]};
           v.as<Address::value_type>() != addr) {
    auto node{_turnouts.extract(addr)};
    node.key() = v.as<Address::value_type>();
    // Re-insert turnout with new address
    if (auto const ret{_turnouts.insert(move(node))}; ret.inserted) {
      it = ret.position;                  // Update iterator
      nvs.erase(addr);                    // Erase old address
      addr = v.as<Address::value_type>(); // Update address
    }
    // Insertion failed
    else
      return std::unexpected<std::string>{"500 Internal Server Error"};
  }
  // Address found, just update turnout
  else
    it->second.fromJsonDocument(doc); // Update iterator

  nvs.set(addr, it->second);

  return {};
}

/// \todo document
[[noreturn]] void Service::taskFunction(void*) {
  switch (state.load()) {
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
    default: assert(false); break;
  }
  LOGI_TASK_DESTROY();
}

/// \todo document
void Service::operationsLoop() {
  while (state.load() == State::DCCOperations) {
    operationsLocos();
    operationsTurnouts();
    operationsBiDi();
    vTaskDelay(pdMS_TO_TICKS(task.timeout));

    //
    if (!empty(_cv_request_deque)) return serviceLoop();
  }

  // wait for task to get deleted
  while (xTaskGetHandle("drv::out::track::dcc"))
    vTaskDelay(pdMS_TO_TICKS(task.timeout));
}

/// Currently fills message buffer between 25 and 50%
void Service::operationsLocos() {
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
        if (_nvs.loco_flags & z21::MmDccSettings::Flags::RepeatHfx) {
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
void Service::operationsTurnouts() {
  for (auto const tick{xTaskGetTickCount()}; auto& [addr, turnout] : _turnouts)
    // Handle turnout timeouts
    if (turnout.timeout_tick && tick >= turnout.timeout_tick) {
      turnout.timeout_tick = 0u;
      bool const p{turnout.position == z21::TurnoutInfo::Position::P1};
      sendToFront(make_basic_accessory_packet(
                    {addr, Address::BasicAccessory}, maybeInvertR(p), false),
                  _nvs.accy_packet_count);
    }
}

/// \todo document
void Service::operationsBiDi() {
  drv::out::track::RxQueue::value_type item;

  while (xQueueReceive(drv::out::track::rx_queue.handle, &item, 0u)) {
    auto const addr{decode_address(item.packet)};

    for (bidi::Dissector dissector{item.datagram, addr};
         auto const& dg : dissector) {
      //
      if (auto pom{get_if<bidi::app::Pom>(&dg)}) {
        if (!empty(_cv_pom_request_deque)) {
          if (decode_instruction(item.packet) == Instruction::CvLong) {
            auto const off{addr.type == Address::ExtendedLoco ||
                           addr.type == Address::BasicAccessory ||
                           addr.type == Address::ExtendedAccessory};
            auto const cv_addr{(item.packet[1uz + off] & 0b11u) << 8u |
                               item.packet[2uz + off]};
            if (auto const& req{_cv_pom_request_deque.front()};
                req.addr == addr && req.cv_addr == cv_addr) {
              cvAck(cv_addr, pom->d);
              _cv_pom_request_deque.pop_front();
            }
          }
        }
      }
      //
      else if (auto dyn{get_if<bidi::app::Dyn>(&dg)}) {
        if (auto const it{_locos.find(addr)}; it != cend(_locos)) {
          it->second.bidi.loco_address = addr;
          auto const bidi_before{it->second.bidi};
          switch (dyn->x) {
            // Speed (<=255)
            case 0u:
              it->second.bidi.options = static_cast<z21::RailComData::Options>(
                (it->second.bidi.options &
                 ~(z21::RailComData::Options::Speed2 |
                   z21::RailComData::Options::Speed1)) |
                z21::RailComData::Options::Speed1);
              it->second.bidi.speed = dyn->d;
              break;
            // Speed (>255)
            case 1u:
              it->second.bidi.options = static_cast<z21::RailComData::Options>(
                (it->second.bidi.options &
                 ~(z21::RailComData::Options::Speed2 |
                   z21::RailComData::Options::Speed1)) |
                z21::RailComData::Options::Speed2);
              it->second.bidi.speed = dyn->d;
              break;
            // QoS
            case 7u:
              it->second.bidi.options = static_cast<z21::RailComData::Options>(
                it->second.bidi.options | z21::RailComData::Options::QoS);
              it->second.bidi.qos = dyn->d;
              break;
          }
          if (it->second.bidi != bidi_before) broadcastRailComData(addr);
        }
      }
    }
  }

  // Handle POM timeouts
  if (!empty(_cv_pom_request_deque) &&
      xTaskGetTickCount() > _cv_pom_request_deque.front().timeout_tick) {
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
      state.compare_exchange_strong(expected, State::Suspending)) {

    // wait for task to get deleted
    while (xTaskGetHandle("drv::out::track::dcc"))
      vTaskDelay(pdMS_TO_TICKS(task.timeout));

    // switch to serv mode
    expected = State::Suspended;
    if (!state.compare_exchange_strong(expected, State::DCCService))
      assert(false);

    // then create
    LOGI_TASK_CREATE(drv::out::track::dcc::task);
  }

  auto const& req{_cv_request_deque.front()};
  auto const cv_addr{req.cv_addr};
  auto const byte{req.byte ? serviceWrite(cv_addr, *req.byte)
                           : serviceRead(cv_addr)};
  _cv_request_deque.pop_front();

  // wait for task to get deleted
  while (xTaskGetHandle("drv::out::track::dcc"))
    vTaskDelay(pdMS_TO_TICKS(task.timeout));

  // send reply
  if (byte) cvAck(cv_addr, *byte);
  else cvNack();
}

/// \todo document
std::optional<uint8_t> Service::serviceRead(uint16_t cv_addr) {
  std::optional<uint8_t> byte{};

  // Nothing
  if (_nvs.programming_type == z21::CommonSettings::ProgrammingType::Nothing)
    return std::nullopt;

  // Byte verify only
  if (_nvs.programming_type == z21::CommonSettings::ProgrammingType::ByteOnly) {
    for (auto i{0u}; i < std::numeric_limits<uint8_t>::max(); ++i) {
      sendToFront(make_cv_access_long_verify_service_packet(cv_addr, i),
                  _nvs.program_packet_count);
      if (serviceReceiveBit() == true) return i;
    }
  }

  // Bit verify
  if (_nvs.programming_type & z21::CommonSettings::ProgrammingType::BitOnly) {
    for (uint8_t i{0u}; i < CHAR_BIT; ++i)
      sendToFront(make_cv_access_long_verify_service_packet(
                    cv_addr, _nvs.bit_verify_to_1, i),
                  _nvs.program_packet_count);
    byte = serviceReceiveByte();

    // Only
    if (_nvs.programming_type == z21::CommonSettings::ProgrammingType::BitOnly)
      return byte;
  }

  // Bit and byte verify
  if (_nvs.programming_type == z21::CommonSettings::ProgrammingType::Both &&
      byte) {
    sendToFront(make_cv_access_long_verify_service_packet(cv_addr, *byte),
                _nvs.program_packet_count);
    if (serviceReceiveBit() == true) return byte;
  }

  return std::nullopt;
}

/// \todo document
std::optional<uint8_t> Service::serviceWrite(uint16_t cv_addr, uint8_t byte) {
  sendToFront(make_cv_access_long_write_service_packet(cv_addr, byte),
              _nvs.program_packet_count);

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
      byte |= static_cast<uint8_t>((*bit == _nvs.bit_verify_to_1) << i);
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
  if (!loco_addr) return {};
  else {
    std::lock_guard lock{_internal_mutex};
    auto& loco{getOrInsertLoco(loco_addr)};
    mem::nvs::Locos nvs;
    nvs.set(loco_addr, loco);
    return loco;
  }
}

/// \todo document
void Service::locoDrive(uint16_t loco_addr,
                        z21::LocoInfo::SpeedSteps speed_steps,
                        uint8_t rvvvvvvv) {
  // Broadcast
  if (!loco_addr) switch (speed_steps) {
      case z21::LocoInfo::DCC14:
        return sendToFront(
          make_speed_and_direction_packet(basicOrExtendedLocoAddress(loco_addr),
                                          (rvvvvvvv & 0x80u) >> 2u | // R
                                            (rvvvvvvv & 0x0Fu)),     // GGGG
          _nvs.program_packet_count);
      case z21::LocoInfo::DCC28:
        return sendToFront(
          make_speed_and_direction_packet(basicOrExtendedLocoAddress(loco_addr),
                                          (rvvvvvvv & 0x80u) >> 2u // R
                                            | (rvvvvvvv & 0x1Fu)), // G-GGGG
          _nvs.program_packet_count);
      case z21::LocoInfo::DCC128:
        return sendToFront(make_advanced_operations_speed_packet(
                             basicOrExtendedLocoAddress(loco_addr), rvvvvvvv),
                           _nvs.program_packet_count);
      default: return;
    }
  //
  else {
    std::lock_guard lock{_internal_mutex};
    auto& loco{getOrInsertLoco(loco_addr)};

    //
    if (loco.speed_steps == speed_steps && loco.rvvvvvvv == rvvvvvvv) return;
    loco.speed_steps = speed_steps;
    loco.rvvvvvvv = rvvvvvvv;

    //
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
    auto& loco{getOrInsertLoco(loco_addr)};

    //
    state = (~mask & loco.f31_0) | (mask & state);
    if (loco.f31_0 == state) return;
    loco.f31_0 = state;

    // Higher functions don't get repeated, send them now
    if (mask >= (1u << 13u) &&
        !(_nvs.loco_flags & z21::MmDccSettings::Flags::RepeatHfx)) {
      if (mask & (0xFFu << 13u))
        sendToBack(make_feature_expansion_f20_f13_packet(
          basicOrExtendedLocoAddress(loco_addr), loco.f31_0 >> 13u));
      if (mask & (0xFFu << 21u))
        sendToBack(make_feature_expansion_f28_f21_packet(
          basicOrExtendedLocoAddress(loco_addr), loco.f31_0 >> 21u));
    }

    //
    mem::nvs::Locos nvs;
    nvs.set(loco_addr, loco);
  }

  //
  broadcastLocoInfo(loco_addr);
}

/// \todo document
z21::LocoInfo::Mode Service::locoMode(uint16_t loco_addr) {
  return locoInfo(loco_addr).mode;
}

/// \todo document
void Service::locoMode(uint16_t, z21::LocoInfo::Mode mode) {
  if (mode == z21::LocoInfo::MM) LOGW("MM not supported");
}

/// \todo document
void Service::broadcastLocoInfo(uint16_t loco_addr) {
  _z21_dcc_service->broadcastLocoInfo(loco_addr);
}

/// \todo document
z21::TurnoutInfo Service::turnoutInfo(uint16_t accy_addr) {
  std::lock_guard lock{_internal_mutex};
  auto& turnout{getOrInsertTurnout(accy_addr)};
  mem::nvs::Turnouts nvs;
  nvs.set(accy_addr, turnout);
  return turnout;
}

/// \todo document
z21::AccessoryInfo Service::accessoryInfo(uint16_t accy_addr) {
  LOGW("accessoryInfo not implemented");
  return {};
}

/// \todo document
// P ^= R in DCC
// P0 -> diverging / left / stop(red)
// P1 -> normal / right / proceed(green)
void Service::turnout(uint16_t accy_addr, bool p, bool a, bool q) {
  sendToFront(make_basic_accessory_packet(
                {accy_addr, Address::BasicAccessory}, maybeInvertR(p), a),
              _nvs.accy_packet_count);

  {
    std::lock_guard lock{_internal_mutex};
    auto& turnout{getOrInsertTurnout(accy_addr)};

    //
    if (!a) {
      turnout.timeout_tick = 0u;
      return;
    }

    if (turnout.position == static_cast<z21::TurnoutInfo::Position>(1u << p))
      return;
    turnout.position = static_cast<z21::TurnoutInfo::Position>(1u << p);

    //
    if (!(_nvs.accy_flags &
          z21::CommonSettings::ExtFlags::TurnoutTimeoutDisable)) {
      auto const timeout{(_nvs.accy_switch_time + 10u) * 10u};
      turnout.timeout_tick = xTaskGetTickCount() + pdMS_TO_TICKS(timeout);
    }

    mem::nvs::Turnouts nvs;
    nvs.set(accy_addr, turnout);
  }

  //
  broadcastTurnoutInfo(accy_addr);
}

/// \todo document
void Service::accessory(uint16_t accy_addr, uint8_t dddddddd) {
  LOGW("accessory addr %d    dddddddd %d", accy_addr, dddddddd);
}

/// \todo document
z21::TurnoutInfo::Mode Service::turnoutMode(uint16_t accy_addr) {
  return turnoutInfo(accy_addr).mode;
}

/// \todo document
void Service::turnoutMode(uint16_t, z21::TurnoutInfo::Mode mode) {
  if (mode == z21::TurnoutInfo::MM) LOGW("MM not supported");
}

/// \todo document
void Service::broadcastTurnoutInfo(uint16_t accy_addr) {
  _z21_dcc_service->broadcastTurnoutInfo(accy_addr);
}

/// \todo document
void Service::broadcastExtAccessoryInfo(uint16_t accy_addr) {
  _z21_dcc_service->broadcastExtAccessoryInfo(accy_addr);
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
              _nvs.program_packet_count);

  _cv_pom_request_deque.push_back(
    {.timeout_tick = xTaskGetTickCount() + pdMS_TO_TICKS(500u), // See RCN-217
     .addr = loco_addr,
     .cv_addr = cv_addr});

  /// \todo reset loco prio here

  // Mandatory delay
  vTaskDelay(
    pdMS_TO_TICKS((_nvs.program_packet_count + 1u) * 10u)); // ~10ms per packet
}

/// \todo document
void Service::cvPomWrite(uint16_t loco_addr, uint16_t cv_addr, uint8_t byte) {
  sendToFront(make_cv_access_long_write_packet(
                basicOrExtendedLocoAddress(loco_addr), cv_addr, byte),
              _nvs.program_packet_count);

  // Mandatory delay
  vTaskDelay(
    pdMS_TO_TICKS((_nvs.program_packet_count + 1u) * 10u)); // ~10ms per packet
}

/// \todo document
void Service::cvPomAccessoryRead(uint16_t accy_addr, uint16_t cv_addr, bool) {
  if (full(_cv_pom_request_deque)) return cvNack();

  sendToFront(make_cv_access_long_verify_packet(
                {.value = accy_addr, .type = Address::BasicAccessory}, cv_addr),
              _nvs.program_packet_count);

  // Dummy CV7 write ensures we aren't receiving app:pom replies to different CV
  // addresses when reading multiple values in row. According to RCN-226 all CV7
  // PoM access are to be ignored by all decoders.
  sendToFront(make_cv_access_long_write_packet(
    {.value = accy_addr, .type = Address::BasicAccessory}, 7u, 0u));

  _cv_pom_request_deque.push_back(
    {.timeout_tick = xTaskGetTickCount() + pdMS_TO_TICKS(500u), // See RCN-217
     .addr = accy_addr,
     .cv_addr = cv_addr});

  // Mandatory delay
  vTaskDelay(
    pdMS_TO_TICKS((_nvs.program_packet_count + 1u) * 10u)); // ~10ms per packet
}

/// \todo document
void Service::cvPomAccessoryWrite(uint16_t accy_addr,
                                  uint16_t cv_addr,
                                  uint8_t byte,
                                  bool) {
  sendToFront(
    make_cv_access_long_write_packet(
      {.value = accy_addr, .type = Address::BasicAccessory}, cv_addr, byte),
    _nvs.program_packet_count);

  // Mandatory delay
  vTaskDelay(
    pdMS_TO_TICKS((_nvs.program_packet_count + 1u) * 10u)); // ~10ms per packet
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
  _nvs.programming_type = nvs.getDccProgrammingType();
  _nvs.program_packet_count = nvs.getDccProgramPacketCount();
  _nvs.bit_verify_to_1 = nvs.getDccBitVerifyTo1();
  _nvs.loco_flags = nvs.getDccLocoFlags();
  _nvs.accy_flags = nvs.getDccAccessoryFlags();
  _nvs.accy_switch_time = nvs.getDccAccessorySwitchTime();
  _nvs.accy_packet_count = nvs.getDccAccessoryPacketCount();

  // Preload
  auto const packet{state.load() == State::DCCOperations ? make_idle_packet()
                                                         : make_reset_packet()};
  while (
    xMessageBufferSpacesAvailable(drv::out::tx_message_buffer.back_handle) >
    drv::out::tx_message_buffer.size * 0.5)
    sendToBack(packet);

  // Create out::track::dcc task
  LOGI_TASK_CREATE(drv::out::track::dcc::task);
}

/// \todo document
void Service::suspend() {
  _priority_count = 0uz;
  _cv_request_deque.clear();
  _cv_pom_request_deque.clear();
  _z21_system_service->broadcastTrackPowerOff();
}

/// \todo document
Loco& Service::getOrInsertLoco(uint16_t loco_addr) {
  auto& loco{_locos[loco_addr]};

  //
  if (empty(loco.name)) loco.name = std::to_string(loco_addr);

  return loco;
}

/// \todo document
Turnout& Service::getOrInsertTurnout(uint16_t accy_addr) {
  auto& turnout{_turnouts[accy_addr]};

  //
  if (empty(turnout.name)) turnout.name = std::to_string(accy_addr);

  //
  if (empty(turnout.group.addresses)) turnout.group.addresses = {accy_addr};

  //
  if (empty(turnout.group.positions))
    turnout.group.positions = {{Turnout::Position::P0},
                               {Turnout::Position::P1}};

  return turnout;
}

/// \todo document
Address Service::basicOrExtendedLocoAddress(Address::value_type addr) const {
  return {.value = addr,
          .type =
            addr <= (_nvs.loco_flags & z21::MmDccSettings::Flags::DccShort127
                       ? 127u
                       : 99u)
              ? Address::BasicLoco
              : Address::ExtendedLoco};
}

/// \todo document
bool Service::maybeInvertR(bool p) const {
  return _nvs.accy_flags & z21::CommonSettings::ExtFlags::AccessoryInvRedGreen
           ? !p
           : p;
}

} // namespace mw::dcc
