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

/// NVS "accessories" namespace
///
/// \file   mem/nvs/accessories.hpp
/// \author Vincent Hamp
/// \date   17/02/2023

#pragma once

#include <dcc/dcc.hpp>
#include "base.hpp"
#include "mw/dcc/accessory.hpp"
#include "utility.hpp"

namespace mem::nvs {

/// Accessories stored in NVS
///
/// \todo not yet implemented
class Accessories : public Base {
public:
  Accessories() : Base{"accessories", NVS_READWRITE} {}
};

} // namespace mem::nvs
