/// NVS "locos" namespace
///
/// \file   mem/nvs/locos.cpp
/// \author Vincent Hamp
/// \date   17/02/2023

#include "locos.hpp"
#include <ArduinoJson.h>
#include <charconv>
#include "log.h"

namespace mem::nvs {

///
dcc::NvLocoBase Locos::get(dcc::Address::value_type addr) const {
  return get(address2key(addr));
}

///
dcc::NvLocoBase Locos::get(std::string const& key) const {
  auto const json{getBlob(key)};
  DynamicJsonDocument doc{1024uz};
  if (auto const err{deserializeJson(doc, data(json), size(json))}) {
    LOGE("Deserialization failed %s", err.c_str());
    return {};
  }
  dcc::NvLocoBase loco;
  loco.fromJsonDocument(doc);
  return loco;
}

///
esp_err_t Locos::set(dcc::Address::value_type addr,
                     dcc::NvLocoBase const& loco) {
  return set(address2key(addr), loco);
}

///
esp_err_t Locos::set(std::string const& key, dcc::NvLocoBase const& loco) {
  auto const doc{loco.toJsonDocument()};
  std::string json;
  json.reserve(1024uz);
  assert(serializeJson(doc, json));
  return setBlob(key, json);
}

///
esp_err_t Locos::erase(dcc::Address::value_type addr) {
  return Base::erase(address2key(addr));
}

///
std::string Locos::address2key(dcc::Address::value_type addr) const {
  return std::to_string(addr);
}

///
dcc::Address::value_type Locos::key2address(std::string_view key) const {
  dcc::Address::value_type addr;
  auto const [ptr, ec]{std::from_chars(std::cbegin(key), std::cend(key), addr)};
  if (ec != std::errc{}) return {};
  return addr;
}

}  // namespace mem::nvs