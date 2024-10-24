// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

///
///
/// \file   z21/init.cpp
/// \author Vincent Hamp
/// \date   15/04/2024

#include "init.hpp"
#include "dcc/service.hpp"
#include "http/sta/server.hpp"
#include "service.hpp"

namespace z21 {

///
esp_err_t init(BaseType_t xCoreID) {
  if (http::sta::server) {
    service = std::make_shared<Service>(xCoreID);
    service->dcc(dcc::service);
    dcc::service->z21(service, service);
    http::sta::server->subscribe({.uri = "/z21/"}, service, &Service::socket);
  }
  return ESP_OK;
}

}  // namespace z21
