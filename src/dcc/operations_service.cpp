#include <dcc/dcc.hpp>
#include <ranges>
#include "log.h"
#include "mem/nvs/locos.hpp"
#include "service.hpp"
#include "utility.hpp"

namespace dcc {

using namespace std::literals;

/// TODO
OperationsService::OperationsService() {
  // TODO could be a transform, but not before the compiler version gets updated
  mem::nvs::Locos nvs;
  for (auto const& entry_info : nvs) {
    auto const addr{nvs.key2address(entry_info.key)};
    static_cast<NvLocoBase&>(_locos[addr]) = nvs.get(entry_info.key);
  }

  LOGI("Got %u locos from NVS", size(_locos));
  for (auto const& [addr, loco] : _locos)
    LOGI("Loco: addr %u, name %s", addr, loco.name.c_str());
}

/// TODO
http::Response OperationsService::locosGetRequest(http::Request const& req) {
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
  // Collection (TODO filters?)
  else {
    DynamicJsonDocument doc{(size(_locos) + 1uz) * 1024uz};
    auto array{doc.to<JsonArray>()};  // Explicitly convert to array
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

/// TODO
http::Response OperationsService::locosDeleteRequest(http::Request const& req) {
  std::lock_guard lock{_internal_mutex};

  // Singleton
  if (auto const addr{uri2address(req.uri).value_or(0u)}) {
    // Erase (doesn't matter if it exists or not)
    _locos.erase(addr);
    mem::nvs::Locos nvs;
    nvs.erase(addr);

    // Invalidate iterator if it points to element being deleted
    if (_it == _locos.find(addr)) _it.reset();
  }
  // Collection (TODO filters?)
  else if (req.uri == "/dcc/locos/"sv) {
    // Erase all
    _locos.clear();
    mem::nvs::Locos nvs;
    nvs.eraseAll();

    // Invalidate iterator
    _it.reset();
  }

  return {};
}

/// TODO
http::Response OperationsService::locosPutRequest(http::Request const& req) {
  // Validate body
  if (!validate_json(req.body))
    return std::unexpected<std::string>{"415 Unsupported Media Type"};

  // Address not found or other characters appended to it
  // We currently only support singleton
  auto const addr{uri2address(req.uri).value_or(0u)};
  if (!addr) return std::unexpected<std::string>{"417 Expectation Failed"};

  // Deserialize (this only creates meta data which points to the source)
  DynamicJsonDocument doc{size(req.body) * 10u};
  if (auto const err{deserializeJson(doc, data(req.body), size(req.body))}) {
    LOGE("Deserialization failed %s", err.c_str());
    return std::unexpected<std::string>{"500 Internal Server Error"};
  }

  // Document contains address and it's not equal to URI
  if (JsonVariantConst doc_addr{doc["address"]};
      doc_addr.as<Address::value_type>() != addr) {
    LOGW("json contains address which is NOT equal to URI");
    // TODO, that sucks because that means we need to change the address... and
    // that has a huge fuck-up potential
  }

  // Get reference to loco, insert if key doesn't exist
  auto& loco{_locos[addr]};
  loco.fromJsonDocument(doc);

  mem::nvs::Locos nvs;
  nvs.set(addr, loco);

  return {};
}

/// TODO
void OperationsService::loop() {
  while (mode.load() == Mode::DCCOperations) {
    execute();
    vTaskDelay(pdMS_TO_TICKS(100u));
  }
}

/// TODO
void OperationsService::execute() {
  // TODO no necessary???
  std::lock_guard lock{_internal_mutex};

  while (xMessageBufferSpacesAvailable(out::tx_message_buffer.back_handle) >
         out::tx_message_buffer.size / 2uz) {
    if (size(_locos)) sendDecoderPackets();
    else sendIdlePacket();
  }

  // TODO REMOVE!!!
  out::track::RxQueue::value_type bidi;
  // xQueueReceive(out::track::rx_queue.handle, &bidi, 0u);
  xQueueReset(out::track::rx_queue.handle);
}

/// Iterate over map and generate DCC packets to push to out::track. This method
/// could push until like 50% of the message buffer is filled and then return?
/// As an optimization we might store a map iterator and only to a couple of
/// packets until we return?
void OperationsService::sendDecoderPackets() {
  if (auto const first{begin(_locos)}; !_it) _it = first;
  else if (auto const last{cend(_locos)}; _it == last) _it = first;
  auto& addr{(*_it)->first};
  auto& loco{(*_it)->second};
  ++(*_it);

  auto packet{
    dcc::make_advanced_operations_speed_packet(addr, loco.dir, loco.speed)};
  xMessageBufferSend(
    out::tx_message_buffer.back_handle, data(packet), size(packet), 0u);

  packet = make_function_group_f4_f0_packet(addr, loco.functions & 0x1Fu);
  xMessageBufferSend(
    out::tx_message_buffer.back_handle, data(packet), size(packet), 0u);

  packet = make_function_group_f8_f5_packet(addr, loco.functions >> 5u & 0xFu);
  xMessageBufferSend(
    out::tx_message_buffer.back_handle, data(packet), size(packet), 0u);

  packet = make_function_group_f12_f9_packet(addr, loco.functions >> 9u & 0xFu);
  xMessageBufferSend(
    out::tx_message_buffer.back_handle, data(packet), size(packet), 0u);
}

/// TODO
void OperationsService::sendIdlePacket() const {
  static constexpr auto idle_packet{dcc::make_idle_packet()};
  xMessageBufferSend(out::tx_message_buffer.back_handle,
                     data(idle_packet),
                     size(idle_packet),
                     0u);
}

}  // namespace dcc