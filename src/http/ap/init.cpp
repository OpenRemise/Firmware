// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize HTTP server for access point
///
/// \file   http/ap/init.cpp
/// \author Vincent Hamp
/// \date   01/03/2023

#include "init.hpp"
#include <memory>
#include "server.hpp"

namespace http::ap {

namespace {

std::unique_ptr<Server> server;

}  // namespace

///
esp_err_t init() {
  server.reset(new Server{wifi::ap_records_queue.handle});
  return ESP_OK;
}

}  // namespace http::ap