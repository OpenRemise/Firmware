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

#pragma once

#include <ArduinoJson.h>
#include <dcc/dcc.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <z21/z21.hpp>
#include <ztl/string.hpp>

namespace mw::dcc {

using namespace ::dcc;
namespace z21 = ::z21;

/// \todo document
struct SystemState : z21::SystemState {
  JsonDocument toJsonDocument() const;
};

} // namespace mw::dcc
