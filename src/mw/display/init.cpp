// Copyright (C) 2026 Vincent Hamp
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

///
///
/// \file   mw/display/init.cpp
/// \author Vincent Hamp
/// \date   08/01/2026

#include "init.hpp"
#include <memory>
#include "intf/http/sta/server.hpp"
#include "service.hpp"

namespace mw::display {

namespace {

std::unique_ptr<Service> service;

} // namespace

/// \todo document
esp_err_t init() {
  if (intf::http::sta::server) {
    service = std::make_unique<Service>([]() -> std::string {
      return *intf::http::sta::server->sysGetRequest({});
    });
  }
  return ESP_OK;
}

} // namespace mw::display
