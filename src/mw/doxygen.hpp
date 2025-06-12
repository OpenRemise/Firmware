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

/// Middleware documentation
///
/// \file   mw/doxygen.hpp
/// \author Vincent Hamp
/// \date   10/06/2025

#pragma once

namespace mw {

/// \page page_mw Middlewares
/// \details
/// The middleware modules are the connections between interfaces and drivers.
///
// clang-format off
/// \page page_mw Middleware
/// \details
/// | Chapter               | Namespace            | Content                                                                                                     |
/// | --------------------- | -------------------- | ----------------------------------------------------------------------------------------------------------- |
/// | \subpage page_mw_dcc  | \ref mw::dcc "dcc"   | [DCC](https://github.com/ZIMO-Elektronik/DCC) operation and service mode, command generation, BiDi decoding |
/// | \subpage page_mw_ota  | \ref mw::ota "ota"   | OTA firmware update (WebSocket service)                                                                     |
/// | \subpage page_mw_z21  | \ref mw::z21 "z21"   | [Z21](https://github.com/ZIMO-Elektronik/Z21) server (UDP and WebSocket service)                            |
/// | \subpage page_mw_zimo | \ref mw::zimo "zimo" | ZIMO specific (USB and WebSocket services)                                                                  |
// clang-format on
/// \page page_mw Middleware
/// \details
///
/// <div class="section_buttons">
/// | Previous           | Next             |
/// | :----------------- | ---------------: |
/// | \ref page_intf_usb | \ref page_mw_dcc |
/// </div>

} // namespace mw
