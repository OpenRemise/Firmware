#include "init.hpp"
#include <memory>
#include "http/sta/server.hpp"
#include "service.hpp"
#include "z21/service.hpp"

namespace dcc {

///
esp_err_t init(BaseType_t xCoreID) {
  if (http::sta::server) {
    service = std::make_shared<Service>(xCoreID);
    http::sta::server->subscribe({.uri = "/dcc/locos/", .method = HTTP_DELETE},
                                 service,
                                 &Service::locosDeleteRequest);
    http::sta::server->subscribe({.uri = "/dcc/locos/", .method = HTTP_GET},
                                 service,
                                 &Service::locosGetRequest);
    http::sta::server->subscribe({.uri = "/dcc/locos/", .method = HTTP_PUT},
                                 service,
                                 &Service::locosPutRequest);
  }
  return ESP_OK;
}

}  // namespace dcc