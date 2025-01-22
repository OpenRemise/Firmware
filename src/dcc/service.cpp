// Copyright (C) 2024 Vincent Hamp
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
#include "log.h"
#include "mem/nvs/locos.hpp"
#include "mem/nvs/settings.hpp"
#include "utility.hpp"

namespace dcc {

using namespace std::literals;

/// \todo document
Service::Service(BaseType_t xCoreID) {
  mem::nvs::Locos nvs;
  for (auto const& entry_info : nvs) {
    auto const addr{nvs.key2address(entry_info.key)};
    dynamic_cast<NvLocoBase&>(_locos[addr]) = nvs.get(entry_info.key);
  }

  LOGI("Got %u locos from NVS", size(_locos));
  for (auto const& [addr, loco] : _locos)
    LOGI("Loco: addr %u, name %s", addr, loco.name.c_str());

  if (!xTaskCreatePinnedToCore(make_tramp(this, &Service::taskFunction),
                               task.name,
                               task.stack_size,
                               NULL,
                               task.priority,
                               &task.handle,
                               xCoreID))
    assert(false);
}

/// \todo document
Service::~Service() {
  if (task.handle) vTaskDelete(task.handle);
}

/// \todo document
void Service::z21(std::shared_ptr<z21::server::intf::System> z21_system_service,
                  std::shared_ptr<z21::server::intf::Dcc> z21_dcc_service) {
  _z21_system_service = z21_system_service;
  _z21_dcc_service = z21_dcc_service;
}

/// \todo document
/// \todo filters?
http::Response Service::locosGetRequest(http::Request const& req) {
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
http::Response Service::locosDeleteRequest(http::Request const& req) {
  std::lock_guard lock{_internal_mutex};

  // Singleton
  if (auto const addr{uri2address(req.uri).value_or(0u)}) {
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
http::Response Service::locosPutRequest(http::Request const& req) {
  // Validate body
  if (!validate_json(req.body))
    return std::unexpected<std::string>{"415 Unsupported Media Type"};

  // Address not found or other characters appended to it
  // We currently only support singleton
  auto const addr{uri2address(req.uri).value_or(0u)};
  if (!addr) return std::unexpected<std::string>{"417 Expectation Failed"};

  // Deserialize
  JsonDocument doc;
  if (auto const err{deserializeJson(doc, req.body)}) {
    LOGE("Deserialization failed %s", err.c_str());
    return std::unexpected<std::string>{"500 Internal Server Error"};
  }

  // Document contains address and it's not equal to URI
  if (JsonVariantConst doc_addr{doc["address"]};
      doc_addr.as<Address::value_type>() != addr) {
    LOGW("json contains address which is NOT equal to URI");
    /// \todo CHANGE ADDRESS OF EXISTING LOCO HERE!!!
  }

  std::lock_guard lock{_internal_mutex};

  // Get reference to loco, insert if key doesn't exist
  auto& loco{_locos[addr]};
  loco.fromJsonDocument(doc);

  mem::nvs::Locos nvs;
  nvs.set(addr, loco);

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
      default: LOGI_TASK_SUSPEND(task.handle); break;
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
  while (eTaskGetState(out::track::dcc::task.handle) != eSuspended)
    vTaskDelay(pdMS_TO_TICKS(task.timeout));
}

/// Currently fills message buffer between 25 and 50%
void Service::operationsDcc() {
  // Less than 50% space available
  if (xMessageBufferSpacesAvailable(out::tx_message_buffer.back_handle) <
      out::tx_message_buffer.size * 0.5)
    return;

  /// \todo necessary?
  std::lock_guard lock{_internal_mutex};

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

      if (xMessageBufferSpacesAvailable(out::tx_message_buffer.back_handle) <
          out::tx_message_buffer.size * 0.25)
        return;
    }
    //
    else {
      for (auto& [addr, loco] : _locos) {
        //
        if (_priority_count % loco.priority) continue;

        // Speed and direction
        switch (loco.speed_steps) {
          case z21::LocoInfo::DCC14: [[fallthrough]]; //  break;
          case z21::LocoInfo::DCC28: [[fallthrough]]; // break;
          case z21::LocoInfo::DCC128:
            sendToBack(
              make_advanced_operations_speed_packet(addr, loco.rvvvvvvv));
            break;
        }

        // Lower functions
        sendToBack(make_function_group_f4_f0_packet(addr, loco.f31_0 & 0x1Fu));
        sendToBack(
          make_function_group_f8_f5_packet(addr, loco.f31_0 >> 5u & 0xFu));
        sendToBack(
          make_function_group_f12_f9_packet(addr, loco.f31_0 >> 9u & 0xFu));

        // Higher functions?

        loco.priority = std::clamp<decltype(loco.priority)>(
          loco.priority + 1u, Loco::min_priority, Loco::max_priority);

        if (xMessageBufferSpacesAvailable(out::tx_message_buffer.back_handle) <
            out::tx_message_buffer.size * 0.25)
          return;
      }

      _priority_count = std::clamp<decltype(_priority_count)>(
        _priority_count + 1u, Loco::min_priority, Loco::max_priority);
    }
  }
}

/// \todo document
void Service::operationsBiDi() {
  out::track::RxQueue::value_type item;
  while (xQueueReceive(out::track::rx_queue.handle, &item, 0u)) {
    // Currently only care for loco addresses
    auto const addr{decode_address(item.packet)};
    if (addr.type != Address::Short && addr.type != Address::Long) continue;

    // Channel 1 (which I don't care about currently)
    std::span const ch1{cbegin(item.datagram), cbegin(item.datagram) + 2};

    // Channel 2
    std::span const ch2{cbegin(item.datagram) + 2, cend(item.datagram)};
    // No data
    if (std::ranges::all_of(ch2, [](uint8_t b) { return !b; })) continue;
    // Invalid data or ACK
    /// \todo remove that later, not caring for acks is just temporarily!
    if (std::ranges::any_of(ch2, [](uint8_t b) {
          return (b && std::popcount(b) != CHAR_BIT / 2) ||
                 (b == dcc::bidi::acks[0uz] || b == dcc::bidi::acks[1uz]);
        }))
      continue;

    // Make data
    auto data{bidi::make_data(bidi::decode_datagram(item.datagram))};

    // Remove channel 1 address part (bits 48-36)
    data &= 0xF'FFFF'FFFFull;

    switch (data >> 32u) {
      // app:pom
      case 0u:
        if (!empty(_cv_pom_request_deque)) {

          auto const off{addr.type == Address::Long};

          if (decode_instruction(item.packet) == Instruction::CvLong) {
            auto const cv_addr{(item.packet[1uz + off] & 0b11u) << 8u |
                               item.packet[2uz]};

            if (auto const& req{_cv_pom_request_deque.front()};
                req.addr == addr && req.cv_addr == cv_addr) {
              cvAck(cv_addr, static_cast<uint8_t>(data >> 24u));
              _cv_pom_request_deque.pop_front();
            }
          }
        }
        break;

      // app:adr_high
      case 1u: break;

      // app:adr_low
      case 2u: break;

      // app:ext
      case 3u: break;

      // app:info
      case 4u: break;

      // app:dyn
      case 7u:
        // might contain another one of those
        break;

      // app:xpom
      case 8u: break;

      // app:xpom
      case 9u: break;

      // app:xpom
      case 10u: break;

      // app:xpom
      case 11u: break;

      // app:CV-auto
      case 12u: break;

      // app:block
      case 13u: break;

      // app:zeit
      case 14u: break;
    }
  }

  //
  if (!empty(_cv_pom_request_deque) &&
      xTaskGetTickCount() > _cv_pom_request_deque.front().then) {
    _cv_pom_request_deque.pop_front();
    cvNack();
  }
}

/// \todo document
void Service::serviceLoop() {
  if (empty(_cv_request_deque)) return;

  /// \todo oh god please make this safer...
  /// it changes from opmode to serv...
  if (auto expected{State::DCCOperations};
      state.compare_exchange_strong(expected, State::Suspend)) {

    // wait for task to get suspended
    while (eTaskGetState(out::track::dcc::task.handle) != eSuspended)
      vTaskDelay(pdMS_TO_TICKS(task.timeout));

    // switch to serv mode
    expected = State::Suspended;
    if (!state.compare_exchange_strong(expected, State::DCCService))
      assert(false);

    // then resume
    LOGI_TASK_RESUME(out::track::dcc::task.handle);
  }

  auto const& req{_cv_request_deque.front()};
  auto const cv_addr{req.cv_addr};
  auto const byte{req.byte ? serviceWrite(cv_addr, *req.byte)
                           : serviceRead(cv_addr)};
  _cv_request_deque.pop_front();

  // wait for task to get suspended
  while (eTaskGetState(out::track::dcc::task.handle) != eSuspended)
    vTaskDelay(pdMS_TO_TICKS(task.timeout));

  // send reply
  if (byte) cvAck(cv_addr, *byte);
  else cvNack();
}

/// \todo document
std::optional<uint8_t> Service::serviceRead(uint16_t cv_addr) {
  std::optional<uint8_t> byte{};

  mem::nvs::Settings nvs;
  auto const programming_type{nvs.getDccProgrammingType()};
  auto const program_packet_count{nvs.getDccProgramPacketCount()};
  auto const bit_verify_to_1{nvs.getDccBitVerifyTo1()};
  nvs.~Settings();

  // Nothing
  if (programming_type == 0x00u) return std::nullopt;

  // Byte verify only
  if (programming_type == 0x02u) {
    for (auto i{0u}; i < std::numeric_limits<uint8_t>::max(); ++i) {
      sendToFront(make_cv_access_long_verify_service_packet(cv_addr, i),
                  program_packet_count);
      if (serviceReceiveBit() == true) return i;
    }
  }

  // Bit verify
  if (programming_type & 0x01u) {
    for (uint8_t i{0u}; i < CHAR_BIT; ++i)
      sendToFront(
        make_cv_access_long_verify_service_packet(cv_addr, bit_verify_to_1, i),
        program_packet_count);
    byte = serviceReceiveByte(bit_verify_to_1);

    // Only
    if (programming_type == 0x01u) return byte;
  }

  // Bit and byte verify
  if (programming_type == 0x03u && byte) {
    sendToFront(make_cv_access_long_verify_service_packet(cv_addr, *byte),
                program_packet_count);
    if (serviceReceiveBit() == true) return byte;
  }

  return std::nullopt;
}

/// \todo document
std::optional<uint8_t> Service::serviceWrite(uint16_t cv_addr, uint8_t byte) {
  sendToFront(make_cv_access_long_write_service_packet(cv_addr, byte),
              programPacketCount());

  if (serviceReceiveBit() == true) return byte;

  return std::nullopt;
}

/// \todo document
/// Depending on the DCC settings we might need to wait a long ass time...
std::optional<bool> Service::serviceReceiveBit() {
  bool bit;
  if (xMessageBufferReceive(out::rx_message_buffer.handle,
                            &bit,
                            sizeof(bit),
                            pdMS_TO_TICKS(std::numeric_limits<uint8_t>::max() *
                                          10u)) == sizeof(bit))
    return bit;
  else return std::nullopt;
}

/// \todo document
std::optional<uint8_t> Service::serviceReceiveByte(bool bit_verify_to_1) {
  uint8_t byte{};
  for (auto i{0uz}; i < CHAR_BIT; ++i)
    if (auto const bit{serviceReceiveBit()})
      byte |= static_cast<uint8_t>((*bit == bit_verify_to_1) << i);
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
    while (!xMessageBufferSend(
      out::tx_message_buffer.front_handle, data(packet), size(packet), 0u));
}

/// \todo document
void Service::sendToBack(Packet const& packet, size_t n) {
  for (auto i{0uz}; i < n; ++i)
    while (!xMessageBufferSend(
      out::tx_message_buffer.back_handle, data(packet), size(packet), 0u));
}

/// \todo document
z21::LocoInfo Service::locoInfo(uint16_t loco_addr) {
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
  {
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
  {
    std::lock_guard lock{_internal_mutex};
    auto& loco{_locos[loco_addr]};

    //
    state = (~mask & loco.f31_0) | (mask & state);
    if (loco.f31_0 == state) return;
    loco.f31_0 = state;

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

  auto const program_packet_count{programPacketCount()};
  sendToFront(make_cv_access_long_verify_packet(loco_addr, cv_addr),
              program_packet_count);

  _cv_pom_request_deque.push_back(
    {.then = xTaskGetTickCount() + pdMS_TO_TICKS(500u), // See RCN-217
     .addr = loco_addr,
     .cv_addr = cv_addr});

  /// \todo reset loco prio here

  // Mandatory delay
  vTaskDelay(
    pdMS_TO_TICKS((program_packet_count + 1u) * 10u)); // ~10ms per packet
}

/// \todo document
void Service::cvPomWrite(uint16_t loco_addr, uint16_t cv_addr, uint8_t byte) {
  auto const program_packet_count{programPacketCount()};
  sendToFront(make_cv_access_long_write_packet(loco_addr, cv_addr, byte),
              program_packet_count);

  // Mandatory delay
  vTaskDelay(
    pdMS_TO_TICKS((program_packet_count + 1u) * 10u)); // ~10ms per packet
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
void Service::cvNackShortCircuit() {
  _z21_dcc_service->cvNackShortCircuit();
  LOGI("cvNackShortCircuit");
}

/// \todo document
void Service::cvNack() {
  _z21_dcc_service->cvNack();
  LOGI("cvNack");
}

/// \todo document
void Service::cvAck(uint16_t cv_addr, uint8_t byte) {
  _z21_dcc_service->cvAck(cv_addr, byte);
  LOGI("cvAck %d %d", cv_addr, byte);
}

/// \todo document
void Service::resume() {
  auto const packet{state.load() == State::DCCOperations ? make_idle_packet()
                                                         : make_reset_packet()};
  while (xMessageBufferSpacesAvailable(out::tx_message_buffer.back_handle) >
         out::tx_message_buffer.size * 0.5)
    sendToBack(packet);
  LOGI_TASK_RESUME(out::track::dcc::task.handle);
}

/// \todo document
void Service::suspend() {
  _priority_count = 0uz;
  _cv_request_deque.clear();
  _cv_pom_request_deque.clear();
  _z21_system_service->broadcastTrackPowerOff();
}

/// \todo document
uint8_t Service::programPacketCount() const {
  mem::nvs::Settings nvs;
  return nvs.getDccProgramPacketCount();
}

} // namespace dcc