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

/// HTTP and WebSocket endpoints
///
/// \file   http/endpoints.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <esp_http_server.h>
#include <concepts>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <ztl/fail.hpp>
#include "log.h"
#include "message.hpp"
#include "request.hpp"
#include "response.hpp"
#include "utility.hpp"

namespace http {

///
class Endpoints {
  using key_type = httpd_uri_t;

  /// Mapped type for HTTP requests
  using sync_mapped_type = std::vector<std::function<Response(Request const&)>>;

  /// Mapped type for WebSockets
  using async_mapped_type = std::vector<std::function<esp_err_t(Message&)>>;

public:
  /// \todo document
  template<typename T, typename F>
  void subscribe(key_type const& key, std::shared_ptr<T> t, F&& f) {
    if constexpr (std::invocable<typename signature<F>::type, Request const&>)
      _sync_map[key].push_back([t, f](auto&&... args) {
        return std::invoke(f, *t, std::forward<decltype(args)>(args)...);
      });
    else if constexpr (std::invocable<typename signature<F>::type, Message&>)
      _async_map[key].push_back([t, f](auto&&... args) {
        return std::invoke(f, *t, std::forward<decltype(args)>(args)...);
      });
    else ztl::fail();
  }

protected:
  /// \todo document
  Response syncResponse(httpd_req_t* req) {
    static constexpr auto chunk_size{16384uz};

    auto const key{req2key(req)};
    auto const it{_sync_map.find(key)};
    if (it == cend(_sync_map))
      return std::unexpected<std::string>{"501 Not Implemented"};

    // Request body can be red in chunks which avoids triggering the servers
    // receive timeout
    Request r{.uri = std::string(req->uri),
              .body = std::string(req->content_len, '\0')};
    int bytes_red{};
    while (bytes_red < req->content_len) {
      if (auto const tmp{
            httpd_req_recv(req, data(r.body) + bytes_red, chunk_size)};
          tmp > 0)
        bytes_red += tmp;
      else return std::unexpected<std::string>{"500 Internal Server Error"};
    }

    /// \todo properly iterating over vector...
    return it->second[0uz](r);
  }

  /// \todo document
  esp_err_t asyncResponse(httpd_req_t* req) {
    auto const key{req2key(req)};
    auto const it{_async_map.find(key)};
    if (it == cend(_async_map)) return ESP_FAIL;
    httpd_ws_frame_t frame{};
    if (httpd_ws_recv_frame(req, &frame, 0uz)) return ESP_FAIL;

    // WebSocket frame must be red in one go
    Message msg{.sock_fd = httpd_req_to_sockfd(req),
                .type = frame.type,
                .payload = std::vector<uint8_t>(frame.len)};
    if (frame.len) {
      frame.payload = data(msg.payload);
      if (httpd_ws_recv_frame(req, &frame, frame.len)) return ESP_FAIL;
    }

    return it->second[0uz](msg);
  }

private:
  /// \todo document
  httpd_uri_t req2key(httpd_req_t* req) const {
    return {.uri = req->uri,
            .method = static_cast<httpd_method_t>(req->method)};
  }

  /// \todo document
  struct key_compare {
    bool operator()(key_type const& lhs, key_type const& rhs) const {
      return lhs.method != rhs.method
               ? lhs.method < rhs.method
               : strncmp(lhs.uri,
                         rhs.uri,
                         std::min(strlen(lhs.uri), strlen(rhs.uri))) < 0;
    }
  };

  std::map<key_type, sync_mapped_type, key_compare> _sync_map;
  std::map<key_type, async_mapped_type, key_compare> _async_map;
};

} // namespace http