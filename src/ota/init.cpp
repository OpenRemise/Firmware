// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

///
///
/// \file   ota/init.cpp
/// \author Vincent Hamp
/// \date   13/06/2023

#include "init.hpp"
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <memory>
#include "http/sta/server.hpp"
#include "log.h"
#include "service.hpp"

namespace ota {

namespace {

std::shared_ptr<Service> service;

}  // namespace

/// \todo document
esp_err_t init(BaseType_t xCoreID) {
  esp_partition_t const* boot_partition{esp_ota_get_boot_partition()};
  esp_partition_t const* running_partition{esp_ota_get_running_partition()};
  if (boot_partition != running_partition)
    LOGW("Configured OTA boot and running partition differs. This can happen "
         "if either the OTA boot data or preferred boot image become corrupted "
         "somehow.");

  if (http::sta::server) {
    service = std::make_shared<Service>(xCoreID);
    http::sta::server->subscribe({.uri = "/ota/"}, service, &Service::socket);
  }

  return ESP_OK;
}

}  // namespace ota