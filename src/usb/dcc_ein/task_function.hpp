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

/// DCC_EIN protocol task function
///
/// \file   usb/dcc_ein/task_function.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <optional>
#include <span>

namespace usb::dcc_ein {

std::optional<dcc::Packet> receive_dcc_packet();
void task_function(void*);

}  // namespace usb::dcc_ein