/// Initialize
///
/// \file   settings/init.cpp
/// \author Vincent Hamp
/// \date   15/05/2023

#include "init.hpp"
#include <memory>
#include "http/sta/server.hpp"
#include "log.h"
#include "service.hpp"

namespace settings {

namespace {

std::shared_ptr<Service> service;

}  // namespace

/// TODO
esp_err_t init() {
  using http::sta::server;

  if (server) {
    service = std::make_shared<Service>();
    server->subscribe(
      {.uri = "/settings/", .method = HTTP_GET}, service, &Service::getRequest);
    server->subscribe({.uri = "/settings/", .method = HTTP_POST},
                      service,
                      &Service::postRequest);
  }

  return ESP_OK;
}

}  // namespace settings