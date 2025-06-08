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

/// ULF_DCC_EIN task function
///
/// \file   mw/ulf/dcc_ein/task_function.hpp
/// \author Vincent Hamp
/// \date   04/05/2025

#pragma once

#include <optional>
#include <ulf/dcc_ein.hpp>

namespace mw::ulf::dcc_ein {

std::optional<::dcc::Packet> receive_dcc_packet();
void task_function(void*);

} // namespace mw::ulf::dcc_ein
