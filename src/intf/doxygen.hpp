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

/// Interface documentation
///
/// \file   intf/doxygen.hpp
/// \author Vincent Hamp
/// \date   10/06/2025

#pragma once

namespace intf {

/// \page page_intf Interfaces
/// \details
/// The interface modules contain the connections to the outside world.
/// Currently, these are lwIP (over WiFi), UDP, and USB.
///
// clang-format off
/// \page page_intf Interface
/// \details
/// | Chapter                 | Namespace | Content                                                                                                                                                                                                                           |
/// | ----------------------- | --------- | ---------------------------------------------------------------------------------- |
/// | \subpage page_intf_http | \ref http | Access point (AP) and station (STA) HTTP servers                                   |
/// | \subpage page_intf_mdns | \ref mdns | Register mDNS services                                                             |
/// | \subpage page_intf_udp  | \ref udp  | Create and bind UDP socket                                                         |
/// | \subpage page_intf_usb  | \ref usb  | Create [CDC](https://en.wikipedia.org/wiki/USB_communications_device_class) device |
// clang-format on
/// \page page_intf Interface
/// \details
///
/// <div class="section_buttons">
/// | Previous                | Next                |
/// | :---------------------- | ------------------: |
/// | \ref page_api_reference | \ref page_intf_http |
/// </div>

} // namespace intf
