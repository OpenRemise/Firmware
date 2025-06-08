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
#include "service.hpp"

namespace mw::decup {

namespace {

std::shared_ptr<Service> service;

} // namespace

/// \todo document
esp_err_t init() {
  if (intf::http::sta::server) {
    service = std::make_shared<Service>();
    intf::http::sta::server->subscribe(
      {.uri = "/decup/zpp/"}, service, &Service::zppSocket);
    intf::http::sta::server->subscribe(
      {.uri = "/decup/zsu/"}, service, &Service::zsuSocket);
  }
  return ESP_OK;
}

} // namespace mw::decup
