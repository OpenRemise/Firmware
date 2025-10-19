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

/// HTTP documentation
///
/// \file   intf/http/doxygen.hpp
/// \author Vincent Hamp
/// \date   19/02/2023

#pragma once

namespace intf::http {

/// \page page_intf_http HTTP
/// \details \tableofcontents
/// The HTTP module contains the implementation of the web server. Depending on
/// the operating mode (access point or station), either a server for the
/// captive portal or one for the REST APIs is created. Which server is
/// ultimately started depends on whether a network SSID is stored in the
/// settings. If no settings are available yet, the access point server is
/// started, otherwise the station server is started.
///
/// \section section_intf_http_ap Access Point
/// The access point server provides the [captive
/// portal](https://github.com/OpenRemise/Firmware/raw/master/src/intf/http/ap/captive_portal.html)
/// for initial network configuration. Two endpoints are created for this
/// purpose:
/// - /save/
///   - POST
/// - /*
///   - GET
///
/// \subsection subsection_intf_http_ap_wildcard /*
/// The wildcard GET endpoint redirects all requests to the captive portal. The
/// string containing the complete HTML code is generated at runtime using
/// [std::format_to_n](https://en.cppreference.com/w/cpp/utility/format/format_to_n.html).
/// In addition to a number of (pre)settings, it also contains a list of
/// available WiFi networks.
///
/// \subsection subsection_intf_http_ap_save /save/
/// The POST endpoint receives the settings of the captive portal when saving in
/// the form of query parameters. The settings are stored in the NVS namespace
/// `settings`. Afterwards the board reboots.
///
/// \section section_intf_http_sta Station
/// The server for station mode provides a whole range of different endpoints
/// that can be used in normal operation.
/// - /dcc/
///   - GET
///   - POST
/// - /dcc/locos/
///   - DELETE
///   - GET
///   - PUT
/// - /dcc/turnouts/
///   - DELETE
///   - GET
///   - PUT
/// - /settings/
///   - GET
///   - POST
/// - /sys/
///   - GET
/// - /*
///   - GET
///
/// WebSockets?
///
/// \subsection subsection_intf_http_sta_endpoints Endpoints
/// The Endpoints class allows you to set up subscriptions to different
/// endpoints. For such a subscription, the request type and endpoint name must
/// be specified. Furthermore, a distinction is made between synchronous (HTTP)
/// and asynchronous (WebSocket) connections.
///
/// Both subscription types are stored in a std::map with a custom key
/// comparator.
///
/// <div class="section_buttons">
/// | Previous       | Next               |
/// | :------------- | -----------------: |
/// | \ref page_intf | \ref page_intf_dns |
/// </div>

} // namespace intf::http
