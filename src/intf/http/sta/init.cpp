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

/// Initialize HTTP server for station
///
/// \file   intf/http/sta/init.cpp
/// \author Vincent Hamp
/// \date   01/03/2023

#include "init.hpp"
#include <memory>
#include <span>
#include "server.hpp"

namespace intf::http::sta {

/// \todo document
esp_err_t init() {
  server = std::make_shared<Server>();
  server->subscribe({.uri = "/settings/", .method = HTTP_GET},
                    server,
                    &Server::settingsGetRequest);
  server->subscribe({.uri = "/settings/", .method = HTTP_POST},
                    server,
                    &Server::settingsPostRequest);
  server->subscribe(
    {.uri = "/sys/", .method = HTTP_GET}, server, &Server::sysGetRequest);
  return ESP_OK;
}

} // namespace intf::http::sta
