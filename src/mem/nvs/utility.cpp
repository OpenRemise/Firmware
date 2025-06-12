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

/// NVS utilities
///
/// \file   mem/nvs/utility.cpp
/// \author Vincent Hamp
/// \date   12/06/2025

#pragma once

namespace mem::nvs {

/// Convert address to key
///
/// \param  addr  Address
/// \return Key
std::string address2key(dcc::Address::value_type addr) {
  return std::to_string(addr);
}

/// Convert key to address
///
/// \param  key Key
/// \return Address
dcc::Address::value_type key2address(std::string_view key) {
  dcc::Address::value_type addr;
  auto const [ptr, ec]{std::from_chars(std::cbegin(key), std::cend(key), addr)};
  if (ec != std::errc{}) return {};
  return addr;
}

} // namespace mem::nvs
