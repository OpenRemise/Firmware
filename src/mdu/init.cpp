#include "init.hpp"
#include <memory>
#include "http/sta/server.hpp"
#include "service.hpp"

namespace mdu {

namespace {

std::shared_ptr<Service> service;

}  // namespace

///
esp_err_t init() {
  using http::sta::server;

  if (server) {
    service = std::make_shared<Service>();
    server->subscribe(
      {.uri = "/mdu/firmware/"}, service, &Service::firmwareSocket);
    server->subscribe({.uri = "/mdu/zpp/"}, service, &Service::zppSocket);
  }

  return ESP_OK;
}

}  // namespace mdu