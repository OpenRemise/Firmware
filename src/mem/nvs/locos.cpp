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

/// Get loco by address
///
/// \param  addr  Address
/// \return Loco
mw::dcc::NvLocoBase Locos::get(dcc::Address::value_type addr) const {
  return get(address2key(addr));
}

/// Get loco by key
///
/// \param  addr  key
/// \return Loco
mw::dcc::NvLocoBase Locos::get(std::string const& key) const {
  auto const json{getBlob(key)};
  JsonDocument doc;
  if (auto const err{deserializeJson(doc, json)}) {
    LOGE("Deserialization failed %s", err.c_str());
    return {};
  }
  return mw::dcc::NvLocoBase{doc};
}

/// Set loco from address
///
/// \param  addr                          Address
/// \param  loco                          Loco
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Locos::set(dcc::Address::value_type addr,
                     mw::dcc::NvLocoBase const& loco) {
  return set(address2key(addr), loco);
}

/// Set loco from key
///
/// \param  key                           Key
/// \param  loco                          Loco
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Locos::set(std::string const& key, mw::dcc::NvLocoBase const& loco) {
  auto const doc{loco.toJsonDocument()};
  std::string json;
  json.reserve(1024uz);
  if (!serializeJson(doc, json)) assert(false);
  return setBlob(key, json);
}

/// Erase loco from address
///
/// \param  addr                  Address
/// \retval ESP_OK                Erase operation was successful
/// \retval ESP_FAIL              Internal error
/// \retval ESP_ERR_NVS_NOT_FOUND Requested key doesn't exist
esp_err_t Locos::erase(dcc::Address::value_type addr) {
  return Base::erase(address2key(addr));
}

} // namespace mem::nvs
