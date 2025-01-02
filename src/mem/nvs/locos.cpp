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

/// \todo document
dcc::NvLocoBase Locos::get(dcc::Address::value_type addr) const {
  return get(address2key(addr));
}

/// \todo document
dcc::NvLocoBase Locos::get(std::string const& key) const {
  auto const json{getBlob(key)};
  JsonDocument doc;
  if (auto const err{deserializeJson(doc, json)}) {
    LOGE("Deserialization failed %s", err.c_str());
    return {};
  }
  dcc::NvLocoBase loco;
  loco.fromJsonDocument(doc);
  return loco;
}

/// \todo document
esp_err_t Locos::set(dcc::Address::value_type addr,
                     dcc::NvLocoBase const& loco) {
  return set(address2key(addr), loco);
}

/// \todo document
esp_err_t Locos::set(std::string const& key, dcc::NvLocoBase const& loco) {
  auto const doc{loco.toJsonDocument()};
  std::string json;
  json.reserve(1024uz);
  if (!serializeJson(doc, json)) assert(false);
  return setBlob(key, json);
}

/// \todo document
esp_err_t Locos::erase(dcc::Address::value_type addr) {
  return Base::erase(address2key(addr));
}

/// \todo document
std::string Locos::address2key(dcc::Address::value_type addr) const {
  return std::to_string(addr);
}

/// \todo document
dcc::Address::value_type Locos::key2address(std::string_view key) const {
  dcc::Address::value_type addr;
  auto const [ptr, ec]{std::from_chars(std::cbegin(key), std::cend(key), addr)};
  if (ec != std::errc{}) return {};
  return addr;
}

} // namespace mem::nvs