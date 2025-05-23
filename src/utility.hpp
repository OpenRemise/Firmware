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

/// Utility functions
///
/// \file   utility.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_heap_caps.h>
#include <bit>
#include <charconv>
#include <dcc/dcc.hpp>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <span>
#include <string_view>
#include "http/message.hpp"
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

namespace detail {

template<std::unsigned_integral T>
constexpr T prime{sizeof(T) <= 4uz ? 16777619ull : 1099511628211ull};

template<std::unsigned_integral T>
constexpr T offset{sizeof(T) <= 4uz ? 2166136261ull : 14695981039346656037ull};

} // namespace detail

// http://www.isthe.com/chongo/tech/comp/fnv/index.html
template<std::unsigned_integral T = uint32_t>
constexpr T fnv1a(uint8_t byte, T hash = ::detail::offset<T>) {
  return (hash ^ byte) * ::detail::prime<T>;
}

template<std::unsigned_integral T = uint32_t>
constexpr T fnv1a(std::span<char const> str) {
  return std::accumulate(cbegin(str),
                         cend(str),
                         ::detail::offset<T>,
                         [](T a, char b) { return fnv1a(b, a); });
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
template<typename T>
using unique_caps_ptr = std::unique_ptr<T, decltype(heap_caps_free)*>;

/// \todo document
template<typename T>
constexpr auto make_unique_caps(size_t size, uint32_t caps) {
  return unique_caps_ptr<T>{std::bit_cast<T*>(heap_caps_malloc(size, caps)),
                            heap_caps_free};
}

/// \todo document
uint32_t http_receive_timeout2ms();

/// \todo document
template<typename F, typename... Ts>
auto invoke_on_core(BaseType_t core_id, F&& f, Ts&&... ts) {
  using R = decltype(f(std::forward<Ts>(ts)...));

  // Pinned and current core are the same
  if (core_id == xPortGetCoreID())
    return std::invoke(std::forward<F>(f), std::forward<Ts>(ts)...);
  // Pinned core is different, return type is void
  else if constexpr (constexpr auto default_stacksize{4096uz};
                     std::is_void_v<R>) {
    // Create tuple to pass to task
    std::tuple<F, std::tuple<Ts...>> t{
      std::forward<F>(f), std::tuple<Ts...>{std::forward<Ts>(ts)...}};

    // Create task and wait for it's deletion
    TaskHandle_t handle;
    if (!xTaskCreatePinnedToCore(
          [](void* pv) {
            auto& _t{*static_cast<decltype(t)*>(pv)};
            std::apply(std::get<0uz>(_t), std::get<1uz>(_t));
            vTaskDelete(NULL);
          },
          NULL,
          default_stacksize,
          &t,
          ESP_TASK_PRIO_MAX - 1u,
          &handle,
          core_id))
      assert(false);
    while (eTaskGetState(handle) < eDeleted) vTaskDelay(1u);
  }
  // Pinned core is different, return type isn't void
  else {
    // Create tuple to pass to task
    std::tuple<R, F, std::tuple<Ts...>> t{
      {}, std::forward<F>(f), std::tuple<Ts...>{std::forward<Ts>(ts)...}};

    // Create task and wait for it's deletion
    TaskHandle_t handle;
    if (!xTaskCreatePinnedToCore(
          [](void* pv) {
            auto& _t{*static_cast<decltype(t)*>(pv)};
            std::get<0uz>(_t) =
              std::apply(std::get<1uz>(_t), std::get<2uz>(_t));
            vTaskDelete(NULL);
          },
          NULL,
          default_stacksize,
          &t,
          ESP_TASK_PRIO_MAX - 1u,
          &handle,
          core_id))
      assert(false);
    while (eTaskGetState(handle) < eDeleted) vTaskDelay(1u);

    return std::get<0uz>(t);
  }
}

/// \todo document
template<typename... Ts>
auto httpd_sess_trigger_close(Ts&&... ts) {
  return httpd_sess_trigger_close(http::handle, std::forward<Ts>(ts)...);
}

/// \todo document
inline auto httpd_queue_work(http::Message* msg) {
  return httpd_queue_work(
    http::handle,
    [](void* arg) {
      auto msg{std::bit_cast<http::Message*>(arg)};

      // Wrap message in httpd_ws_frame_t
      httpd_ws_frame_t frame{
        .type = msg->type,
        .payload = data(msg->payload),
        .len = size(msg->payload),
      };
      if (auto const err{
            httpd_ws_send_frame_async(http::handle, msg->sock_fd, &frame)})
        LOGE("httpd_ws_send_frame_async failed %s", esp_err_to_name(err));

      // Delete
      delete msg;
    },
    msg);
}
