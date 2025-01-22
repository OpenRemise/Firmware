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
/// \file   mem/nvs/locos.hpp
/// \author Vincent Hamp
/// \date   17/02/2023

#pragma once

#include <dcc/dcc.hpp>
#include "base.hpp"
#include "dcc/loco.hpp"

namespace mem::nvs {

/// Locos stored in NVS
///
/// nvs::Locos allows to store locomotives (i.e. mobile decoders) in a JSON
/// format in the NVS namespace "locos". The decoder address is used as key, the
/// JSON string represents the value. The conversion to and from JSON is handled
/// internally by the [ArduinoJson](https://arduinojson.org/) library. Getters
/// and setters are overloaded and available in `dcc::Address::value_type` or
/// `std::string` variants.
///
/// The two utility functions nvs::Locos::address2key() and
/// nvs::Locos::key2address() are also part of this class. They can be used to
/// convert a `dcc::Address::value_type` into a `std::string` (or vice versa).
class Locos : public Base {
public:
  static std::string address2key(dcc::Address::value_type addr);
  static dcc::Address::value_type key2address(std::string_view key);

  explicit Locos() : Base{"locos", NVS_READWRITE} {}

  dcc::NvLocoBase get(dcc::Address::value_type addr) const;
  dcc::NvLocoBase get(std::string const& key) const;
  esp_err_t set(dcc::Address::value_type addr, dcc::NvLocoBase const& loco);
  esp_err_t set(std::string const& key, dcc::NvLocoBase const& loco);
  esp_err_t erase(dcc::Address::value_type addr);
};

} // namespace mem::nvs