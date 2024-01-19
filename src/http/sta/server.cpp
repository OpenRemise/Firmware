/// HTTP server for station
///
/// \file   http/sta/server.cpp
/// \author Vincent Hamp
/// \date   01/03/2023

#include "server.hpp"
#include <driver/gpio.h>
#include <esp_vfs.h>
#include <dcc/dcc.hpp>
#include <gsl/util>
#include <ztl/string.hpp>
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "utility.hpp"

namespace http::sta {

using namespace std::literals;

/// TODO
Server::Server() {
  assert(_file_buffer_ptr);

  //
  mem::nvs::Settings nvs;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.core_id = 0;
  config.max_uri_handlers = 16u;
  config.lru_purge_enable = true;
  config.recv_wait_timeout = nvs.getHttpReceiveTimeout();
  config.send_wait_timeout = nvs.getHttpTransmitTimeout();
  config.keep_alive_enable = true;
  config.uri_match_fn = httpd_uri_match_wildcard;
  ESP_ERROR_CHECK(httpd_start(&_server, &config));

  //
  httpd_uri_t uri{.uri = "/dcc/locos/*",
                  .method = HTTP_DELETE,
                  .handler = make_tramp(this, &Server::deleteHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/dcc/locos/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::getHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/dcc/locos/*",
         .method = HTTP_PUT,
         .handler = make_tramp(this, &Server::putPostHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/dcc/service/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::getHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/dcc/service/*",
         .method = HTTP_PUT,
         .handler = make_tramp(this, &Server::putPostHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/settings/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::getHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/settings/*",
         .method = HTTP_POST,
         .handler = make_tramp(this, &Server::putPostHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/sys/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::getHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/sys/*",
         .method = HTTP_POST,
         .handler = make_tramp(this, &Server::putPostHandler)};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/mdu/firmware/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::mduFirmwareWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/mdu/zpp/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::mduZppWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/ota/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::otaWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/zusi/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::zusiWsHandler),
         .is_websocket = true,
         .handle_ws_control_frames = true};
  httpd_register_uri_handler(_server, &uri);

  //
  uri = {.uri = "/*",
         .method = HTTP_GET,
         .handler = make_tramp(this, &Server::wildcardGetHandler)};
  httpd_register_uri_handler(_server, &uri);
}

/// TODO
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

/// TODO
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

/// TODO
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
// clang-format off
#define GENERIC_WS_HANDLER(NAME, URI)                                          \
esp_err_t Server::NAME(httpd_req_t* req) {                                     \
  if (req->method == HTTP_GET) return ESP_OK;                                  \
  else {                                                                       \
    httpd_req_t cpy{.handle = req->handle,                                     \
                    .method = req->method,                                     \
                    .uri = URI,                                                \
                    .content_len = req->content_len,                           \
                    .aux = req->aux,                                           \
                    .user_ctx = req->user_ctx,                                 \
                    .sess_ctx = req->sess_ctx,                                 \
                    .free_ctx = req->free_ctx,                                 \
                    .ignore_sess_ctx_changes = req->ignore_sess_ctx_changes};  \
    return asyncResponse(&cpy);                                                \
  }                                                                            \
}
// clang-format on

GENERIC_WS_HANDLER(mduFirmwareWsHandler, "/mdu/firmware/")
GENERIC_WS_HANDLER(mduZppWsHandler, "/mdu/zpp/")
GENERIC_WS_HANDLER(otaWsHandler, "/ota/")
GENERIC_WS_HANDLER(zusiWsHandler, "/zusi/")

/// TODO
esp_err_t Server::wildcardGetHandler(httpd_req_t* req) {
  LOGD("GET request %s", req->uri);

  struct stat file_stat;
  FILE* fd{};

  // 308 / to index.html
  if (req->uri == "/"sv) {
    httpd_resp_set_status(req, "308 Permanent Redirect");
    httpd_resp_set_hdr(req, "Location", "/index.html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  }
  // SPIFFS
  else if (!stat(req->uri, &file_stat))
    fd = fopen(req->uri, "r");
  // 404
  else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  //
  if (!fd) {
    LOGE("Failed to read existing file");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  gsl::final_action close{[&fd] { fclose(fd); }};

  // Set content type
  std::string_view const uri{req->uri};
  if (uri.ends_with(".css")) httpd_resp_set_type(req, "text/css");
  else if (uri.ends_with(".js")) httpd_resp_set_type(req, "text/javascript");
  else if (uri.ends_with(".otf")) httpd_resp_set_type(req, "font/otf");
  else if (uri.ends_with(".png")) httpd_resp_set_type(req, "image/png");
  else if (uri.ends_with(".svg")) httpd_resp_set_type(req, "image/svg+xml");
  else if (uri.ends_with(".ttf")) httpd_resp_set_type(req, "font/ttf");

  //
  LOGD("Sending file : %s (%ldB)", req->uri, file_stat.st_size);
  size_t chunksize;
  do {
    if (!(chunksize =
            fread(_file_buffer_ptr.get(), sizeof(char), file_buffer_size, fd)))
      continue;
    if (httpd_resp_send_chunk(req, _file_buffer_ptr.get(), chunksize) !=
        ESP_OK) {
      LOGE("File sending failed");
      httpd_resp_send_chunk(req, NULL, 0);  // Abort sending file
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
  } while (chunksize);

  httpd_resp_send_chunk(req, NULL, 0);
  return ESP_OK;
}

}  // namespace http::sta