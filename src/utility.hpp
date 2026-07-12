// Copyright (C) 2025 Vincent Hamp
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

/// General utilities
///
/// \file   utility.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <bit>
#include <charconv>
#include <dcc/dcc.hpp>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <span>
#include <string_view>
#include "intf/http/message.hpp"
#include "log.h"

void esp_delayed_restart();

bool validate_json(std::string_view json);

// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c
template<typename T>
consteval std::string_view type_name() {
  using namespace std;
#if defined(__clang__)
  string_view p{__PRETTY_FUNCTION__};
  return string_view(data(p) + 34, size(p) - 34 - 1);
#elif defined(__GNUC__)
  string_view p{__PRETTY_FUNCTION__};
#  if __cplusplus < 201402
  return string_view(data(p) + 36, size(p) - 36 - 1);
#  else
  return string_view(data(p) + 49, p.find(';', 49) - 49);
#  endif
#elif defined(_MSC_VER)
  string_view p{__FUNCSIG__};
  return string_view(data(p) + 84, size(p) - 84 - 7);
#endif
}

std::optional<dcc::Address::value_type> uri2address(std::string_view uri);

std::optional<dcc::Address> uri2loco_address(std::string_view uri);

/// https://rosettacode.org/wiki/URL_decoding#C
template<std::output_iterator<char> OutputIt>
OutputIt decode_uri(std::string_view uri, OutputIt out) {
  auto first{begin(uri)};
  auto const last{cend(uri)};
  while (first < last) {
    auto c{*first++};
    if (c == '+') c = ' ';
    else if (c == '%') {
      std::from_chars(first, first + 2, c, 16);
      first += 2;
    }
    *out++ = c;
  }
  return out;
}

/// \todo document
uint32_t http_receive_timeout2ms();

/// \todo document
esp_err_t ipc_call_blocking(BaseType_t core_id, esp_err_t (*f)());

/// \todo document
template<typename... Ts>
auto httpd_sess_trigger_close(Ts&&... ts) {
  return httpd_sess_trigger_close(intf::http::handle, std::forward<Ts>(ts)...);
}

/// \todo document
inline auto httpd_queue_work(intf::http::Message* msg) {
  return httpd_queue_work(
    intf::http::handle,
    [](void* arg) {
      auto msg{static_cast<intf::http::Message*>(arg)};

      // Wrap message in httpd_ws_frame_t
      httpd_ws_frame_t frame{
        .type = msg->type,
        .payload = data(msg->payload),
        .len = size(msg->payload),
      };
      if (auto const err{httpd_ws_send_frame_async(
            intf::http::handle, msg->sock_fd, &frame)})
        LOGD("httpd_ws_send_frame_async failed %s", esp_err_to_name(err));

      // Delete
      delete msg;
    },
    msg);
}
