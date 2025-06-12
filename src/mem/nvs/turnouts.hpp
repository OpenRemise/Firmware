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
/// \file   mem/nvs/turnouts.hpp
/// \author Vincent Hamp
/// \date   12/06/2025

#pragma once

#include <dcc/dcc.hpp>
#include "base.hpp"
#include "mw/dcc/turnout.hpp"
#include "utility.hpp"

namespace mem::nvs {

/// Turnouts stored in NVS
///
/// \todo not yet implemented
class Turnouts : public Base {
public:
  Turnouts() : Base{"turnouts", NVS_READWRITE} {}

  mw::dcc::NvTurnoutBase get(dcc::Address::value_type addr) const;
  mw::dcc::NvTurnoutBase get(std::string const& key) const;
  esp_err_t set(dcc::Address::value_type addr,
                mw::dcc::NvTurnoutBase const& loco);
  esp_err_t set(std::string const& key, mw::dcc::NvTurnoutBase const& loco);
  esp_err_t erase(dcc::Address::value_type addr);
};

} // namespace mem::nvs
