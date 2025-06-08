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

/// HTTP websocket message
///
/// \file   http/message.hpp
/// \author Vincent Hamp
/// \date   12/04/2023

#pragma once

#include <esp_http_server.h>
#include <vector>

namespace http {

struct Message {
  int sock_fd;
  httpd_ws_type_t type;
  std::vector<uint8_t> payload;
};

} // namespace http
