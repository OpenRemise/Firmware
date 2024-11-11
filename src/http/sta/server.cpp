// Copyright (C) 2024 Vincent Hamp
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

/// HTTP server for station
///
/// \file   http/sta/server.cpp
/// \author Vincent Hamp
/// \date   01/03/2023

#include "server.hpp"
#include <ArduinoJson.h>
#include <driver/gpio.h>
#include <esp_app_desc.h>
#include <dcc/dcc.hpp>
#include <gsl/util>
#include <magic_enum.hpp>
#include <ztl/string.hpp>
#include "analog/convert.hpp"
#include "frontend_embeds.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "utility.hpp"

namespace http::sta {

using namespace std::literals;

/// Ctor
Server::Server() {
  //
  mem::nvs::Settings nvs;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.stack_size = stack_size;
  config.core_id = 0;
  config.max_uri_handlers = 16u;
  config.lru_purge_enable = true;
  config.recv_wait_timeout = nvs.getHttpReceiveTimeout();
  config.send_wait_timeout = nvs.getHttpTransmitTimeout();
  config.uri_match_fn = httpd_uri_match_wildcard;
  ESP_ERROR_CHECK(httpd_start(&handle, &config));

  //
  httpd_uri_t uri{.uri = "/dcc/locos/*",
                  .method = HTTP_DELETE,
                  .handler = make_tramp(this, &Server::deleteHandler)};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/dcc/locos/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::getHandler)};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/dcc/locos/*",
         .method = HTTP_PUT,
         .handler = make_tramp(this, &Server::putPostHandler)};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/settings/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::getHandler)};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/settings/*",
         .method = HTTP_POST,
         .handler = make_tramp(this, &Server::putPostHandler)};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/sys/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::getHandler)};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/decup/zpp/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::decupZppWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/decup/zsu/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::decupZsuWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/mdu/zpp/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::mduZppWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/mdu/zsu/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::mduZsuWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/ota/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::otaWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/z21/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::z21WsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/zusi/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::zusiWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(handle, &uri);

  //
  uri = {.uri = "/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::wildcardGetHandler)};
  httpd_register_uri_handler(handle, &uri);
}

/// Dtor
Server::~Server() {
  ESP_ERROR_CHECK(httpd_stop(&handle));
  handle = NULL;
}

/// \todo document
Response Server::settingsGetRequest(Request const& req) {
  mem::nvs::Settings nvs;

  // Read password (and hide it)
  auto sta_pass{nvs.getStationPassword()};
  std::ranges::fill(sta_pass, '*');

  //
  JsonDocument doc;
  doc["sta_mdns"] = nvs.getStationmDNS();
  doc["sta_ssid"] = nvs.getStationSSID();
  doc["sta_pass"] = sta_pass;
  doc["http_rx_timeout"] = nvs.getHttpReceiveTimeout();
  doc["http_tx_timeout"] = nvs.getHttpTransmitTimeout();
  doc["usb_rx_timeout"] = nvs.getUsbReceiveTimeout();
  doc["current_limit"] = std::to_underlying(nvs.getCurrentLimit());
  doc["current_sc_time"] = nvs.getCurrentShortCircuitTime();
  doc["dcc_preamble"] = nvs.getDccPreamble();
  doc["dcc_bit1_dur"] = nvs.getDccBit1Duration();
  doc["dcc_bit0_dur"] = nvs.getDccBit0Duration();
  doc["dcc_bidibit_dur"] = nvs.getDccBiDiBitDuration();
  doc["dcc_prog_type"] = nvs.getDccProgrammingType();
  doc["dcc_strtp_rs_pc"] = nvs.getDccStartupResetPacketCount();
  doc["dcc_cntn_rs_pc"] = nvs.getDccContinueResetPacketCount();
  doc["dcc_prog_pc"] = nvs.getDccProgramPacketCount();
  doc["dcc_verify_bit1"] = nvs.getDccBitVerifyTo1();
  doc["dcc_ack_cur"] = nvs.getDccProgrammingAckCurrent();
  doc["dcc_flags"] = nvs.getDccFlags();
  doc["mdu_preamble"] = nvs.getMduPreamble();
  doc["mdu_ackreq"] = nvs.getMduAckreq();

  //
  std::string json;
  json.reserve(1024uz);
  serializeJson(doc, json);

  return json;
}

/// \todo document
Response Server::settingsPostRequest(Request const& req) {
  LOGI("%s", __PRETTY_FUNCTION__);
  LOGI("uri %s", req.uri.c_str());
  LOGI("body %s", req.body.c_str());

  // Validate body
  if (!validate_json(req.body))
    return std::unexpected<std::string>{"415 Unsupported Media Type"};

  // Deserialize (this only creates meta data which points to the source)
  JsonDocument doc;
  if (auto const err{deserializeJson(doc, req.body)}) {
    LOGE("Deserialization failed %s", err.c_str());
    return std::unexpected<std::string>{"500 Internal Server Error"};
  }

  mem::nvs::Settings nvs;

  if (JsonVariantConst v{doc["sta_mdns"]}; v.is<std::string>())
    if (auto const str{v.as<std::string>()}; nvs.setStationmDNS(str) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["sta_ssid"]}; v.is<std::string>())
    if (auto const str{v.as<std::string>()}; nvs.setStationSSID(str) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["sta_pass"]}; v.is<std::string>())
    if (auto const str{v.as<std::string>()};
        nvs.setStationPassword(str) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["http_rx_timeout"]}; v.is<uint8_t>())
    if (nvs.setHttpReceiveTimeout(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["http_tx_timeout"]}; v.is<uint8_t>())
    if (nvs.setHttpTransmitTimeout(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["usb_rx_timeout"]}; v.is<uint8_t>())
    if (nvs.setUsbReceiveTimeout(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["current_limit"]}; v.is<uint8_t>())
    if (nvs.setCurrentLimit(
          static_cast<out::track::CurrentLimit>(v.as<uint8_t>())) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["current_sc_time"]}; v.is<uint8_t>())
    if (nvs.setCurrentShortCircuitTime(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_preamble"]}; v.is<uint8_t>())
    if (nvs.setDccPreamble(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_bit1_dur"]}; v.is<uint8_t>())
    if (nvs.setDccBit1Duration(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_bit0_dur"]}; v.is<uint8_t>())
    if (nvs.setDccBit0Duration(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_bidibit_dur"]}; v.is<uint8_t>())
    if (nvs.setDccBiDiBitDuration(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_prog_type"]}; v.is<uint8_t>())
    if (nvs.setDccProgrammingType(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_strtp_rs_pc"]}; v.is<uint8_t>())
    if (nvs.setDccStartupResetPacketCount(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_cntn_rs_pc"]}; v.is<uint8_t>())
    if (nvs.setDccContinueResetPacketCount(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_prog_pc"]}; v.is<uint8_t>())
    if (nvs.setDccProgramPacketCount(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_verify_bit1"]}; v.is<bool>())
    if (nvs.setDccBitVerifyTo1(v.as<bool>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_ack_cur"]}; v.is<uint8_t>())
    if (nvs.setDccProgrammingAckCurrent(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["dcc_flags"]}; v.is<uint8_t>())
    if (nvs.setDccFlags(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["mdu_preamble"]}; v.is<uint8_t>())
    if (nvs.setMduPreamble(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  if (JsonVariantConst v{doc["mdu_ackreq"]}; v.is<uint8_t>())
    if (nvs.setMduAckreq(v.as<uint8_t>()) != ESP_OK)
      return std::unexpected<std::string>{"422 Unprocessable Entity"};

  return {};
}

/// \todo document
Response Server::sysGetRequest(Request const& req) {
  using namespace analog;

  //
  JsonDocument doc;

  doc["state"] = magic_enum::enum_name(state.load());

  auto const app_desc{esp_app_get_description()};
  doc["version"] = app_desc->version;
  doc["project_name"] = app_desc->project_name;
  doc["compile_time"] = app_desc->time;
  doc["compile_date"] = app_desc->date;
  doc["idf_version"] = app_desc->idf_ver;

  doc["mdns"] = wifi::mdns_str;
  doc["ip"] = wifi::ip_str;
  doc["mac"] = wifi::mac_str;

  doc["heap"] = esp_get_free_heap_size();
  doc["internal_heap"] = esp_get_free_internal_heap_size();

  if (VoltagesQueue::value_type voltages;
      xQueuePeek(voltages_queue.handle, &voltages, 0u))
    doc["voltage"] =
      measurement2mV(static_cast<VoltageMeasurement>(
                       std::accumulate(cbegin(voltages), cend(voltages), 0) /
                       size(voltages)))
        .value();

  if (CurrentsQueue::value_type currents;
      xQueuePeek(currents_queue.handle, &currents, 0u))
    doc["current"] =
      measurement2mA(static_cast<CurrentMeasurement>(
                       std::accumulate(cbegin(currents), cend(currents), 0) /
                       size(currents)))
        .value();

  if (TemperatureQueue::value_type temp;
      xQueuePeek(temperature_queue.handle, &temp, 0u))
    doc["temperature"] = temp;

  //
  std::string json;
  json.reserve(1024uz);
  serializeJson(doc, json);

  return json;
}

/// \todo document
esp_err_t Server::deleteHandler(httpd_req_t* req) {
  LOGD("DELETE request %s", req->uri);

  //
  if (auto resp{syncResponse(req)}) {
    httpd_resp_send(req, data(*resp), size(*resp));
    return ESP_OK;
  }
  //
  else {
    auto const& status{resp.error()};
    httpd_resp_set_status(req, status.c_str());
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }
}

/// \todo document
esp_err_t Server::getHandler(httpd_req_t* req) {
  LOGD("GET request %s", req->uri);

  //
  if (auto resp{syncResponse(req)}) {
    httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    httpd_resp_send(req, data(*resp), size(*resp));
    return ESP_OK;
  }
  //
  else {
    auto const& status{resp.error()};
    httpd_resp_set_status(req, status.c_str());
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }
}

/// \todo document
esp_err_t Server::putPostHandler(httpd_req_t* req) {
  LOGD("%s request %s", req->method == HTTP_PUT ? "PUT" : "POST", req->uri);

  // No content
  if (!req->content_len) {
    httpd_resp_send_err(req, HTTPD_411_LENGTH_REQUIRED, NULL);
    return ESP_FAIL;
  }
  //
  else if (auto resp{syncResponse(req)}) {
    httpd_resp_send(req, data(*resp), size(*resp));
    return ESP_OK;
  }
  //
  else {
    auto const& status{resp.error()};
    httpd_resp_set_status(req, status.c_str());
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }
}

// https://github.com/espressif/esp-idf/issues/11661
#define GENERIC_WS_HANDLER(NAME, URI)                                          \
  esp_err_t Server::NAME(httpd_req_t* req) {                                   \
    if (req->method == HTTP_GET) {                                             \
      LOGI("WS open");                                                         \
      return ESP_OK;                                                           \
    } else {                                                                   \
      httpd_req_t cpy{.handle = req->handle,                                   \
                      .method = req->method,                                   \
                      .uri = URI,                                              \
                      .content_len = req->content_len,                         \
                      .aux = req->aux,                                         \
                      .user_ctx = req->user_ctx,                               \
                      .sess_ctx = req->sess_ctx,                               \
                      .free_ctx = req->free_ctx,                               \
                      .ignore_sess_ctx_changes =                               \
                        req->ignore_sess_ctx_changes};                         \
      return asyncResponse(&cpy);                                              \
    }                                                                          \
  }

GENERIC_WS_HANDLER(decupZppWsHandler, "/decup/zpp/")
GENERIC_WS_HANDLER(decupZsuWsHandler, "/decup/zsu/")
GENERIC_WS_HANDLER(mduZppWsHandler, "/mdu/zpp/")
GENERIC_WS_HANDLER(mduZsuWsHandler, "/mdu/zsu/")
GENERIC_WS_HANDLER(otaWsHandler, "/ota/")
GENERIC_WS_HANDLER(z21WsHandler, "/z21/")
GENERIC_WS_HANDLER(zusiWsHandler, "/zusi/")

/// \todo document
esp_err_t Server::wildcardGetHandler(httpd_req_t* req) {
  LOGD("GET request %s", req->uri);

  // 308 / to index.html
  if (std::string_view const uri{req->uri}; uri == "/"sv) {
    httpd_resp_set_status(req, "308 Permanent Redirect");
    httpd_resp_set_hdr(req, "Location", "/index.html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  }
  // Frontend (embedded in firmware)
  else if (auto const it{std::ranges::find_if(
             frontend_embeds,
             [uri](auto&& embed) { return uri.contains(embed[0uz]); })};
           it != cend(frontend_embeds)) {
    // Set content type
    if (uri.ends_with(".css")) httpd_resp_set_type(req, "text/css");
    else if (uri.ends_with(".js")) httpd_resp_set_type(req, "text/javascript");
    else if (uri.ends_with(".otf")) httpd_resp_set_type(req, "font/otf");
    else if (uri.ends_with(".png")) httpd_resp_set_type(req, "image/png");
    else if (uri.ends_with(".svg")) httpd_resp_set_type(req, "image/svg+xml");
    else if (uri.ends_with(".ttf")) httpd_resp_set_type(req, "font/ttf");

    // Send file
    auto const [_, start, end]{*it};
    httpd_resp_send(req, start, end - start);

    return ESP_OK;
  }
  // 404
  else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }
}

}  // namespace http::sta