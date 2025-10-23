// Copyright (C) 2025 Vincent Hamp
// Copyright (C) 2025 Franziska Walter
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/// HTTP server for access point
///
/// \file   intf/http/ap/server.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <string>
#include "utility.hpp"

namespace intf::http::ap {

class Server {
public:
  explicit Server();
  ~Server();

private:
  void buildApRecordsStrings();
  void buildGetString();
  void getConfig();
  void setConfig() const;
  esp_err_t rootGetHandler(httpd_req_t* req);
  esp_err_t savePostHandler(httpd_req_t* req);
  esp_err_t wildcardGetHandler(httpd_req_t* req);

  std::string _ap_options_str;
  std::string _ap_records_str;
  std::string _get_str;
  std::string _sta_mdns_str;
  std::string _sta_ssid_str;
  std::string _sta_pass_str;
  std::string _sta_alt_ssid_str;
  std::string _sta_alt_pass_str;
  std::string _sta_ip_str;
  std::string _sta_netmask_str;
  std::string _sta_gateway_str;
};

} // namespace intf::http::ap
