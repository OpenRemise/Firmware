#include "init.hpp"
#include <memory>
#include "http/sta/server.hpp"
#include "service.hpp"

namespace dcc {

namespace {

std::shared_ptr<Service> service;

}  // namespace

///
esp_err_t init() {
  using http::sta::server;

  if (server) {
    service = std::make_shared<Service>();
    server->subscribe({.uri = "/dcc/locos/", .method = HTTP_DELETE},
                      service,
                      &Service::locosDeleteRequest);
    server->subscribe({.uri = "/dcc/locos/", .method = HTTP_GET},
                      service,
                      &Service::locosGetRequest);
    server->subscribe({.uri = "/dcc/locos/", .method = HTTP_PUT},
                      service,
                      &Service::locosPutRequest);
    server->subscribe({.uri = "/dcc/service/", .method = HTTP_GET},
                      service,
                      &Service::serviceGetRequest);
    server->subscribe({.uri = "/dcc/service/", .method = HTTP_PUT},
                      service,
                      &Service::servicePutRequest);
  }

  return ESP_OK;
}

}  // namespace dcc