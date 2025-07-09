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

#include "system_state.hpp"
#include "log.h"

namespace mw::dcc {

/// \todo document
JsonDocument SystemState::toJsonDocument() const {
  JsonDocument doc;
  doc["main_current"] = main_current;
  doc["prog_current"] = prog_current;
  doc["filtered_main_current"] = filtered_main_current;
  doc["temperature"] = temperature;
  doc["supply_voltage"] = supply_voltage;
  doc["vcc_voltage"] = vcc_voltage;
  doc["central_state"] = std::to_underlying(central_state);
  doc["central_state_ex"] = std::to_underlying(central_state_ex);
  doc["capabilities"] = std::to_underlying(capabilities);
  return doc;
}

} // namespace mw::dcc
