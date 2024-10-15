// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// HTTP server for station
///
/// \file   http/sta/server.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <memory>
#include "../endpoints.hpp"
#include "utility.hpp"

namespace http::sta {

///
class Server : public Endpoints {
public:
  Server();
  ~Server();

  Response settingsGetRequest(Request const& req);
  Response settingsPostRequest(Request const& req);
  Response sysGetRequest(Request const& req);

private:
  esp_err_t deleteHandler(httpd_req_t* req);
  esp_err_t getHandler(httpd_req_t* req);
  esp_err_t putPostHandler(httpd_req_t* req);
  esp_err_t mduZppWsHandler(httpd_req_t* req);
  esp_err_t mduZsuWsHandler(httpd_req_t* req);
  esp_err_t otaWsHandler(httpd_req_t* req);
  esp_err_t z21WsHandler(httpd_req_t* req);
  esp_err_t zusiWsHandler(httpd_req_t* req);
  esp_err_t wildcardGetHandler(httpd_req_t* req);

  /// Internal RAM buffer
  unique_caps_ptr<char> _file_buffer_ptr{
    make_unique_caps<char>(file_buffer_size, MALLOC_CAP_INTERNAL)};
};

}  // namespace http::sta