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

/// NVS "turnouts" namespace
///
/// \file   mem/nvs/turnouts.cpp
/// \author Vincent Hamp
/// \date   12/06/2025

#include "turnouts.hpp"
#include <ArduinoJson.h>
#include <charconv>
#include "log.h"

namespace mem::nvs {

/// Get turnout by address
///
/// \param  addr  Address
/// \return Turnout
mw::dcc::NvTurnoutBase Turnouts::get(dcc::Address::value_type addr) const {
  return get(address2key(addr));
}

/// Get turnout by key
///
/// \param  addr  key
/// \return Turnout
mw::dcc::NvTurnoutBase Turnouts::get(std::string const& key) const {
  return {};
}

/// Set turnout from address
///
/// \param  addr                          Address
/// \param  turnout                       Turnout
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Turnouts::set(dcc::Address::value_type addr,
                        mw::dcc::NvTurnoutBase const& turnout) {
  return set(address2key(addr), turnout);
}

/// Set turnout from key
///
/// \param  key                           Key
/// \param  turnout                       Turnout
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Turnouts::set(std::string const& key,
                        mw::dcc::NvTurnoutBase const& turnout) {
  return {};
}

/// Erase turnout from address
///
/// \param  addr                  Address
/// \retval ESP_OK                Erase operation was successful
/// \retval ESP_FAIL              Internal error
/// \retval ESP_ERR_NVS_NOT_FOUND Requested key doesn't exist
esp_err_t Turnouts::erase(dcc::Address::value_type addr) {
  return Base::erase(address2key(addr));
}

} // namespace mem::nvs
