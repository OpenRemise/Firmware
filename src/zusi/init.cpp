// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "init.hpp"
#include <memory>
#include "http/sta/server.hpp"
#include "service.hpp"

namespace zusi {

namespace {

std::shared_ptr<Service> service;

}  // namespace

/// \todo document
esp_err_t init(BaseType_t xCoreID) {
  if (http::sta::server) {
    service = std::make_shared<Service>(xCoreID);
    http::sta::server->subscribe({.uri = "/zusi/"}, service, &Service::socket);
  }
  return ESP_OK;
}

}  // namespace zusi