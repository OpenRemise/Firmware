#include "service_service.hpp"
#include <ranges>
#include "log.h"
#include "utility.hpp"

namespace dcc {

/// TODO
http::Response ServiceService::serviceGetRequest(http::Request const& req) {
  // Unfortunately we have to lock at this point, otherwise it could happen that
  // values get erased under our ass.
  std::lock_guard lock{_internal_mutex};

  DynamicJsonDocument doc{(size(_done) + 1uz) * 32uz};
  JsonObject obj{doc.to<JsonObject>()};  // Explicitly convert to object
  for (auto const& [addr, value] : _done) {
    auto const key{std::to_string(addr + 1u)};
    if (value) doc[key] = *value;
    else doc[key] = nullptr;
  }
  std::string json;
  json.reserve((size(_done) + 1uz) * 32uz);
  serializeJson(doc, json);
  return json;
}

/// TODO
http::Response ServiceService::servicePutRequest(http::Request const& req) {
  // Deserialize (this only creates meta data which points to the source)
  DynamicJsonDocument doc{size(req.body) * 10u};
  if (auto const err{deserializeJson(doc, data(req.body), size(req.body))}) {
    LOGE("Deserialization failed %s", err.c_str());
    return std::unexpected<std::string>{"500 Internal Server Error"};
  }

  // Technically we wouldn't need to lock to keep other references valid
  // https://en.cppreference.com/w/cpp/container
  // But if we were just about to erase an element we could end up in a
  // situation where we mutate an element we immediately erase again once the
  // lock gets released.
  std::lock_guard lock{_internal_mutex};

  //
  auto obj{doc.as<JsonObject>()};
  for (JsonPair const& p : obj) {
    if (auto const [k, v]{jsonPair2keyValue(p)}; k && v) _todo[*k - 1u] = *v;
    else return std::unexpected<std::string>{"417 Expectation Failed"};
  }

  return {};
}

/// TODO
void ServiceService::loop() {
  while (mode.load() == Mode::DCCService) {
    execute();
    vTaskDelay(pdMS_TO_TICKS(100u));
  }

  //
  clear();
}

/// TODO
void ServiceService::execute() {
  // Things todo
  if (!empty(_todo)) sendCvPackets();
  // Neither waiting for ack/nak nor issuing new command
  else if (xMessageBufferIsEmpty(out::tx_message_buffer.back_handle))
    sendResetPacket();
}

/// TODO
void ServiceService::clear() {
  std::lock_guard lock{_internal_mutex};
  _todo.clear();
  _done.clear();
}

/// TODO
void ServiceService::sendCvPackets() {
  // Clear message buffer
  while (!xMessageBufferReset(out::rx_message_buffer.handle))
    vTaskDelay(pdMS_TO_TICKS(20u));
  if (auto const [addr, value]{getTodoThenErase()}; value)
    sendCvWritePackets(addr, value);
  else sendCvReadPackets(addr);
}

/// TODO
void ServiceService::sendCvWritePackets(key_type addr, mapped_type value) {
  // Byte write
  sendPacketNTimes(dcc::make_cv_access_long_write_service_packet(addr, *value),
                   N_);
  if (receiveBit() == true) {
    _done[addr] = value;
    return optionalDelayForAddress(addr);
  }

  LOGE("Timeout");
  _done[addr] = std::nullopt;
}

/// TODO
void ServiceService::sendCvReadPackets(key_type addr) {
  // Bit verify
  for (uint8_t i{0u}; i < CHAR_BIT; ++i)
    sendPacketNTimes(
      dcc::make_cv_access_long_verify_service_packet(addr, true, i), N_);
  auto const value{receiveByte()};

  // Byte verify
  if (value) {
    sendPacketNTimes(
      dcc::make_cv_access_long_verify_service_packet(addr, *value), N_);
    if (receiveBit() == true) {
      _done[addr] = *value;
      return;
    }
  }

  LOGE("Timeout");
  _done[addr] = std::nullopt;
}

/// TODO
void ServiceService::sendResetPacket() const {
  static constexpr auto reset_packet{dcc::make_reset_packet()};
  sendPacketNTimes(reset_packet, 1uz);
}

/// TODO
void ServiceService::sendPacketNTimes(dcc::Packet const& packet,
                                      size_t n) const {
  for (auto i{0uz}; i < n; ++i)
    xMessageBufferSend(
      out::tx_message_buffer.back_handle, data(packet), size(packet), 0u);
}

/// TODO
void ServiceService::optionalDelayForAddress(key_type addr) const {
  if (addr != 8uz - 1uz && addr != 265uz - 1uz) return;
  constexpr auto timeout{2000u};
  constexpr auto loop{10u};
  for (auto i{0uz}; i < loop; ++i) {
    sendResetPacket();
    vTaskDelay(pdMS_TO_TICKS(timeout / loop));
  }
}

/// TODO
std::pair<std::optional<ServiceService::key_type>,
          std::optional<ServiceService::mapped_type>>
ServiceService::jsonPair2keyValue(JsonPair const& p) const {
  return {jsonString2key(p.key()), jsonVariant2value(p.value())};
}

/// TODO
std::optional<ServiceService::key_type>
ServiceService::jsonString2key(JsonString const& str) const {
  auto const first{str.c_str()};
  auto const last{first + str.size()};
  key_type cv_number{};
  auto const [ptr, ec]{std::from_chars(first, last, cv_number)};
  if (ec != std::errc{} || ptr != last || !cv_number) return std::nullopt;
  return cv_number;
}

/// TODO
std::optional<ServiceService::mapped_type>
ServiceService::jsonVariant2value(JsonVariantConst const& v) const {
  // Value must be either null (read)
  if (v.isNull()) return mapped_type{};
  // or uint8_t (write)
  else if (v.is<uint8_t>()) return v.as<uint8_t>();
  else return std::nullopt;
}

/// TODO
ServiceService::value_type ServiceService::getTodoThenErase() {
  std::lock_guard lock{_internal_mutex};
  auto const first{cbegin(_todo)};
  auto const retval{*first};
  _todo.erase(first);
  return retval;
}

/// TODO
std::optional<bool> ServiceService::receiveBit() const {
  bool bit;
  if (xMessageBufferReceive(out::rx_message_buffer.handle,
                            &bit,
                            sizeof(bit),
                            pdMS_TO_TICKS(200u)) == sizeof(bit))
    return bit;
  else return std::nullopt;
}

/// TODO
std::optional<uint8_t> ServiceService::receiveByte() const {
  uint8_t value{};
  for (auto i{0uz}; i < CHAR_BIT; ++i)
    if (auto const bit{receiveBit()}) value |= static_cast<uint8_t>(*bit << i);
    else return std::nullopt;
  return value;
}

}  // namespace dcc