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

///
///
/// \file   mw/ota/init.cpp
/// \author Vincent Hamp
/// \date   13/06/2023

#include "init.hpp"
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <memory>
#include "intf/http/sta/server.hpp"
#include "log.h"
#include "service.hpp"

namespace mw::ota {

namespace {

std::shared_ptr<Service> service;

} // namespace

/// \todo document
esp_err_t init() {
  esp_partition_t const* boot_partition{esp_ota_get_boot_partition()};
  esp_partition_t const* running_partition{esp_ota_get_running_partition()};
  if (boot_partition != running_partition)
    LOGW("Configured OTA boot and running partition differs. This can happen "
         "if either the OTA boot data or preferred boot image become corrupted "
         "somehow.");

  if (intf::http::sta::server) {
    service = std::make_shared<Service>();
    intf::http::sta::server->subscribe(
      {.uri = "/ota/"}, service, &Service::socket);
  }

  return ESP_OK;
}

} // namespace mw::ota
