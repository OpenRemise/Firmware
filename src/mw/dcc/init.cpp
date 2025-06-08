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

#include "init.hpp"
#include <memory>
#include "intf/http/sta/server.hpp"
#include "mw/z21/service.hpp"
#include "service.hpp"

namespace dcc {

/// \todo document
esp_err_t init() {
  if (http::sta::server) {
    service = std::make_shared<Service>();
    http::sta::server->subscribe({.uri = "/dcc/locos/", .method = HTTP_DELETE},
                                 service,
                                 &Service::locosDeleteRequest);
    http::sta::server->subscribe({.uri = "/dcc/locos/", .method = HTTP_GET},
                                 service,
                                 &Service::locosGetRequest);
    http::sta::server->subscribe({.uri = "/dcc/locos/", .method = HTTP_PUT},
                                 service,
                                 &Service::locosPutRequest);
  }
  return ESP_OK;
}

} // namespace dcc
