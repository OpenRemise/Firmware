/// HTTP server for station
///
/// \file   http/sta/server.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <esp_http_server.h>
#include <memory>
#include "endpoints.hpp"
#include "utility.hpp"

namespace http::sta {

///
class Server : public Endpoints {
public:
  Server();

private:
  esp_err_t deleteHandler(httpd_req_t* req);
  esp_err_t getHandler(httpd_req_t* req);
  esp_err_t putPostHandler(httpd_req_t* req);
  esp_err_t mduFirmwareWsHandler(httpd_req_t* req);
  esp_err_t mduZppWsHandler(httpd_req_t* req);
  esp_err_t otaWsHandler(httpd_req_t* req);
  esp_err_t zusiWsHandler(httpd_req_t* req);
  esp_err_t wildcardGetHandler(httpd_req_t* req);

  /// Internal RAM buffer
  unique_caps_ptr<char> _file_buffer_ptr{
    make_unique_caps<char>(file_buffer_size, MALLOC_CAP_INTERNAL)};
  httpd_handle_t _server{};
};

}  // namespace http::sta