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
/// \file   intf/http/ap/server.cpp
/// \author Vincent Hamp
/// \date   01/03/2023

#include "server.hpp"
#include <esp_wifi.h>
#include <fmt/args.h>
#include <fmt/core.h>
#include <freertos/queue.h>
#include <ztl/string.hpp>
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "utility.hpp"

extern "C" {
extern char const _binary_captive_portal_html_start;
extern char const _binary_captive_portal_html_end;
}

namespace intf::http::ap {

/// Ctor
Server::Server() {
  _ap_records_str.reserve(1024uz);
  _ap_options_str.reserve(1024uz);
  _get_str.reserve(8096uz);
  getConfig();
  buildApRecordsStrings();
  buildGetString();

  //
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.core_id = WIFI_TASK_CORE_ID;
  config.lru_purge_enable = true;
  config.keep_alive_enable = true;
  config.uri_match_fn = httpd_uri_match_wildcard;
  ESP_ERROR_CHECK(httpd_start(&handle, &config));

  //
  httpd_uri_t uri{.uri = "/",
                  .method = HTTP_GET,
                  .handler =
                    ztl::make_trampoline(this, &Server::rootGetHandler)};
  ESP_ERROR_CHECK(httpd_register_uri_handler(handle, &uri));

  //
  uri = {.uri = "/save/",
         .method = HTTP_POST,
         .handler = ztl::make_trampoline(this, &Server::savePostHandler)};
  ESP_ERROR_CHECK(httpd_register_uri_handler(handle, &uri));

  //
  uri = {.uri = "/*",
         .method = HTTP_GET,
         .handler = ztl::make_trampoline(this, &Server::wildcardGetHandler)};
  ESP_ERROR_CHECK(httpd_register_uri_handler(handle, &uri));
}

/// Dtor
Server::~Server() {
  ESP_ERROR_CHECK(httpd_stop(&handle));
  handle = NULL;
}

/// \todo document
void Server::buildApRecordsStrings() {
  for (auto const& ap_record : drv::wifi::ap_records) {
    //
    _ap_records_str.append("<li>");
    _ap_records_str.append(std::bit_cast<char const*>(&ap_record.ssid));
    _ap_records_str.append(" ");
    std::array<char, 4uz> tmp{};
    auto const q{std::clamp(2 * (ap_record.rssi + 100), 0, 100)};
    std::to_chars(begin(tmp), end(tmp), q);
    _ap_records_str.append(begin(tmp));
    _ap_records_str.append("% ");
    if (ap_record.authmode != WIFI_AUTH_OPEN) {
      static constexpr auto unicode_lock{"&#x1F512"};
      _ap_records_str.append(unicode_lock);
    }
    _ap_records_str.append("</li>");

    //
    _ap_options_str.append("<option>");
    _ap_options_str.append(std::bit_cast<char const*>(&ap_record.ssid));
    _ap_options_str.append("</option>");
  }
}

/// \todo document
void Server::buildGetString() {
  auto const result{fmt::format_to_n(
    begin(_get_str),
    _get_str.capacity(),
    fmt::runtime({&_binary_captive_portal_html_start,
                  static_cast<size_t>(&_binary_captive_portal_html_end -
                                      &_binary_captive_portal_html_start)}),
    fmt::arg("sta_mdns", _sta_mdns_str),
    fmt::arg("sta_ssid", _sta_ssid_str),
    fmt::arg("ap_options", _ap_options_str),
    fmt::arg("sta_pass", _sta_pass_str),
    fmt::arg("sta_alt_ssid", _sta_alt_ssid_str),
    fmt::arg("sta_alt_pass", _sta_alt_pass_str),
    fmt::arg("sta_ip", _sta_ip_str),
    fmt::arg("sta_netmask", _sta_netmask_str),
    fmt::arg("sta_gateway", _sta_gateway_str),
    fmt::arg("ap_records", _ap_records_str))};
  _get_str.resize(result.size);
}

/// \todo document
void Server::getConfig() {
  mem::nvs::Settings nvs;

  // Read mDNS
  _sta_mdns_str = nvs.getStationmDNS();

  // Read SSID
  _sta_ssid_str = nvs.getStationSSID();

  // Read password
  _sta_pass_str = nvs.getStationPassword();

  // Read alternative SSID
  _sta_alt_ssid_str = nvs.getStationAlternativeSSID();

  // Read alternative password
  _sta_alt_pass_str = nvs.getStationAlternativePassword();

  // Read IP
  _sta_ip_str = nvs.getStationIP();

  // Read netmask
  _sta_netmask_str = nvs.getStationNetmask();

  // Read gateway
  _sta_gateway_str = nvs.getStationGateway();
}

/// \todo document
void Server::setConfig() const {
  mem::nvs::Settings nvs;
  ESP_ERROR_CHECK(nvs.setStationmDNS(_sta_mdns_str));
  ESP_ERROR_CHECK(nvs.setStationSSID(_sta_ssid_str));
  ESP_ERROR_CHECK(nvs.setStationPassword(_sta_pass_str));
  ESP_ERROR_CHECK(nvs.setStationAlternativeSSID(_sta_alt_ssid_str));
  ESP_ERROR_CHECK(nvs.setStationAlternativePassword(_sta_alt_pass_str));
  ESP_ERROR_CHECK(nvs.setStationIP(_sta_ip_str));
  ESP_ERROR_CHECK(nvs.setStationNetmask(_sta_netmask_str));
  ESP_ERROR_CHECK(nvs.setStationGateway(_sta_gateway_str));
}

/// \todo document
esp_err_t Server::rootGetHandler(httpd_req_t* req) {
  LOGD("GET request %s", req->uri);

  buildGetString();
  httpd_resp_send(req, data(_get_str), ssize(_get_str));
  return ESP_OK;
}

/// \todo document
esp_err_t Server::savePostHandler(httpd_req_t* req) {
  LOGD("POST request %s", req->uri);

  assert(req->content_len < CONFIG_HTTPD_MAX_URI_LEN);
  std::array<char, CONFIG_HTTPD_MAX_URI_LEN> stack;

  int len;
  if (len = httpd_req_recv(req, data(stack), req->content_len); len <= 0) {
    httpd_resp_send_err(req, HTTPD_411_LENGTH_REQUIRED, NULL);
    return ESP_FAIL;
  }

  // Inplace decode
  std::string_view content{begin(stack), static_cast<size_t>(len)};
  auto const last{decode_uri(content, begin(stack))};
  std::string_view decoded{begin(stack), last};

  // Find key-value pair positions in query
  auto const sta_mdns_pos{decoded.find("sta_mdns=")};
  assert(sta_mdns_pos != std::string_view::npos && sta_mdns_pos == 0uz);
  auto const sta_ssid_pos{decoded.find("&sta_ssid=")};
  assert(sta_ssid_pos != std::string_view::npos);
  auto const sta_pass_pos{decoded.find("&sta_pass=")};
  assert(sta_pass_pos != std::string_view::npos);
  auto const sta_alt_ssid_pos{decoded.find("&sta_alt_ssid=")};
  assert(sta_alt_ssid_pos != std::string_view::npos);
  auto const sta_alt_pass_pos{decoded.find("&sta_alt_pass=")};
  assert(sta_alt_pass_pos != std::string_view::npos);
  auto const sta_ip_pos{decoded.find("&sta_ip=")};
  assert(sta_ip_pos != std::string_view::npos);
  auto const sta_netmask_pos{decoded.find("&sta_netmask=")};
  assert(sta_netmask_pos != std::string_view::npos);
  auto const sta_gateway_pos{decoded.find("&sta_gateway=")};
  assert(sta_gateway_pos != std::string_view::npos);

  // Build strings from found positions
  _sta_mdns_str.replace(begin(_sta_mdns_str),
                        end(_sta_mdns_str),
                        begin(decoded) + ztl::strlen("sta_mdns="),
                        begin(decoded) + sta_ssid_pos);
  _sta_ssid_str.replace(begin(_sta_ssid_str),
                        end(_sta_ssid_str),
                        begin(decoded) + sta_ssid_pos +
                          ztl::strlen("&sta_ssid="),
                        begin(decoded) + sta_pass_pos);
  _sta_pass_str.replace(begin(_sta_pass_str),
                        end(_sta_pass_str),
                        begin(decoded) + sta_pass_pos +
                          ztl::strlen("&sta_pass="),
                        begin(decoded) + sta_alt_ssid_pos);
  _sta_alt_ssid_str.replace(begin(_sta_alt_ssid_str),
                            end(_sta_alt_ssid_str),
                            begin(decoded) + sta_alt_ssid_pos +
                              ztl::strlen("&sta_alt_ssid="),
                            begin(decoded) + sta_alt_pass_pos);
  _sta_alt_pass_str.replace(begin(_sta_alt_pass_str),
                            end(_sta_alt_pass_str),
                            begin(decoded) + sta_alt_pass_pos +
                              ztl::strlen("&sta_alt_pass="),
                            begin(decoded) + sta_ip_pos);
  _sta_ip_str.replace(begin(_sta_ip_str),
                      end(_sta_ip_str),
                      begin(decoded) + sta_ip_pos + ztl::strlen("&sta_ip="),
                      begin(decoded) + sta_netmask_pos);
  _sta_netmask_str.replace(begin(_sta_netmask_str),
                           end(_sta_netmask_str),
                           begin(decoded) + sta_netmask_pos +
                             ztl::strlen("&sta_netmask="),
                           begin(decoded) + sta_gateway_pos);
  _sta_gateway_str.replace(begin(_sta_gateway_str),
                           end(_sta_gateway_str),
                           begin(decoded) + sta_gateway_pos +
                             ztl::strlen("&sta_gateway="),
                           last);

  // Store values in NVS
  setConfig();

  // https://stackoverflow.com/questions/37337412/should-i-use-a-301-302-or-303-redirect-after-form-submission
  // https://stackoverflow.com/questions/2604530/a-good-way-to-redirect-with-a-post-request
  httpd_resp_set_status(req, "303 See Other");
  httpd_resp_set_hdr(req, "Location", "/");
  httpd_resp_send(req, NULL, 0); // Response body can be empty

  // Restart in 1s
  esp_delayed_restart();

  return ESP_OK;
}

/// \todo document
esp_err_t Server::wildcardGetHandler(httpd_req_t* req) {
  LOGD("GET request %s", req->uri);

  httpd_resp_set_status(req, "302 Found");
  httpd_resp_set_hdr(req, "Location", "/");
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

} // namespace intf::http::ap
