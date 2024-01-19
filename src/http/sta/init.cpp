/// Initialize HTTP server for station
///
/// \file   http/sta/init.cpp
/// \author Vincent Hamp
/// \date   01/03/2023

#include "init.hpp"
#include <memory>
#include <span>
#include "server.hpp"

namespace http::sta {

///
esp_err_t init() {
  server = std::make_shared<Server>();
  return ESP_OK;
}

}  // namespace http::sta