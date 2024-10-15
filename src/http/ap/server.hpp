// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// HTTP server for access point
///
/// \file   http/ap/server.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <string>
#include "utility.hpp"

namespace http::ap {

class Server {
public:
  explicit Server(QueueHandle_t ap_records_queue_handle);
  ~Server();

private:
  void buildApRecordsStrings(QueueHandle_t ap_records_queue_handle);
  void buildGetString();
  void getConfig();
  void setConfig() const;
  esp_err_t wildcardGetHandler(httpd_req_t* req);
  esp_err_t savePostHandler(httpd_req_t* req);

  static constexpr auto unicode_lock_{"&#x1F512"};

  std::string _ap_options_str;
  std::string _ap_records_str;
  std::string _get_str;
  std::string _sta_mdns_str;
  std::string _sta_ssid_str;
  std::string _sta_pass_str;
};

}  // namespace http::ap