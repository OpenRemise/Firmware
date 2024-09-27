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