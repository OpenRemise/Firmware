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
/// The HTTP module contains the implementation of the web server. This web
/// server is accessible in both access point and station modes.
///
/// \section section_intf_http_server Server
/// The server provides a whole range of different regular and websocket
/// endpoints.
///
/// \subsection subsection_intf_http_regular_endpoints Regular endpoints
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
/// \todo Write proper regular endpoint docs
///
/// \subsection subsection_intf_http_websockets Websockets
/// - /ota
///   - GET
/// - /roco/z21
///   - GET
/// - /zimo/decup/zpp
///   - GET
/// - /zimo/decup/zsu
///   - GET
/// - /zimo/mdu/zpp
///   - GET
/// - /zimo/mdu/zsu
///   - GET
/// - /zimo/zusi
///   - GET
///
/// \todo Write proper websocket endpoint docs
///
/// \section section_intf_http_endpoints_class Endpoints class
/// The Endpoints class allows you to set up subscriptions to different
/// endpoints. For such a subscription, the request type and endpoint name must
/// be specified. Furthermore, a distinction is made between synchronous (HTTP)
/// and asynchronous (WebSocket) connections.
///
/// Both subscription types are stored in a `std::map` with a custom key
/// comparator.
///
/// <div class="section_buttons">
/// | Previous           | Next                |
/// | :----------------- | ------------------: |
/// | \ref page_intf_dns | \ref page_intf_mdns |
/// </div>

} // namespace intf::http
