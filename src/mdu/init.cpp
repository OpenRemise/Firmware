// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "init.hpp"
#include <memory>
#include "http/sta/server.hpp"
#include "service.hpp"

namespace mdu {

namespace {

std::shared_ptr<Service> service;

}  // namespace

///
esp_err_t init(BaseType_t xCoreID) {
  if (http::sta::server) {
    service = std::make_shared<Service>(xCoreID);
    http::sta::server->subscribe(
      {.uri = "/mdu/zpp/"}, service, &Service::zppSocket);
    http::sta::server->subscribe(
      {.uri = "/mdu/zsu/"}, service, &Service::zsuSocket);
  }
  return ESP_OK;
}

}  // namespace mdu