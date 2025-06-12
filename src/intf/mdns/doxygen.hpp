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

/// mDNS documentation
///
/// \file   intf/mdns/doxygen.hpp
/// \author Vincent Hamp
/// \date   21/03/2025

#pragma once

namespace intf::mdns {

/// \page page_mdns mDNS
/// \tableofcontents
/// [mDNS](https://en.wikipedia.org/wiki/Multicast_DNS) is a multicast UDP
/// service that is used to provide local network service and host discovery.
/// This module registers those services.
///
/// \section section_mdns_init Initialization
/// \copydetails init
///
/// \note
/// To list mDNS services you can use [avahi](https://github.com/avahi/avahi),
/// which is typically installed by default.
/// ```sh
/// avahi-browse -a
/// ```
///
/// <div class="section_buttons">
/// | Previous       | Next          |
/// | :------------- | ------------: |
/// | \ref page_http | \ref page_udp |
/// </div>

} // namespace intf::mdns
