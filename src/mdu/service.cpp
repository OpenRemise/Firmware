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

#include "service.hpp"
#include <ulf/mdu_ein.hpp>
#include <ztl/utility.hpp>
#include "led/bug.hpp"
#include "log.h"
#include "utility.hpp"

namespace mdu {

using ::ulf::mdu_ein::ack, ::ulf::mdu_ein::nak;

/// \todo document
Service::Service() {
  task.create(ztl::make_trampoline(this, &Service::taskFunction));
}

/// \todo document
Service::~Service() { task.destroy(); }

/// \bug should this broadcast Z21 programming mode?
esp_err_t Service::zppSocket(http::Message& msg) {
  return socket(msg, State::MDUZpp);
}

/// \bug should this broadcast Z21 programming mode?
esp_err_t Service::zsuSocket(http::Message& msg) {
  return socket(msg, State::MDUZsu);
}

/// \bug should this broadcast Z21 programming mode?
esp_err_t Service::socket(http::Message& msg, State mdu_state) {
  //
  if (auto expected{State::Suspended};
      msg.type != HTTPD_WS_TYPE_CLOSE &&
      state.compare_exchange_strong(expected, mdu_state)) {
    _queue.push(std::move(msg));
    LOGI_TASK_RESUME(task.handle);
    LOGI_TASK_RESUME(out::track::mdu::task.handle);
    return ESP_OK;
  }
  //
  else if (state.load() == mdu_state) {
    _queue.push(std::move(msg));
    return ESP_OK;
  }
  //
  else
    return ESP_FAIL;
}

/// \todo document
void Service::taskFunction(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND();
    switch (state.load()) {
      case State::MDUZpp: [[fallthrough]];
      case State::MDUZsu: loop(); break;
      default: assert(false); break;
    }
  }
}

/// \todo document
void Service::loop() {
  led::Bug const led_bug{};
  auto const timeout{http_receive_timeout2ms()};

  for (;;) {
    assert(_queue.size());
    auto const msg{std::move(_queue.front())};
    _queue.pop();

    switch (msg.type) {
      case HTTPD_WS_TYPE_BINARY: _acks = transmit(msg.payload); break;
      case HTTPD_WS_TYPE_CLOSE: LOGI("WebSocket closed"); return close();
      default:
        LOGE("WebSocket packet type neither binary nor close");
        _acks = {ack, nak};
        break;
    }

    if (auto const err{httpd_queue_work(new http::Message{
          .sock_fd = msg.sock_fd,
          .type = HTTPD_WS_TYPE_BINARY,
          .payload = {cbegin(_acks), cend(_acks)},
        })}) {
      LOGE("httpd_queue_work failed %s", esp_err_to_name(err));
      return close();
    }

    TickType_t const then{xTaskGetTickCount() + pdMS_TO_TICKS(timeout)};
    while (empty(_queue))
      if (xTaskGetTickCount() >= then) {
        LOGI("WebSocket timeout");
        if (auto const err{httpd_sess_trigger_close(msg.sock_fd)})
          LOGE("httpd_sess_trigger_close failed %s", esp_err_to_name(err));
        return close();
      }
  }
}

/// \todo document
std::array<uint8_t, 2uz>
Service::transmit(std::vector<uint8_t> const& payload) const {
  //
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(payload),
                     std::min(size(payload), MDU_MAX_PACKET_SIZE),
                     portMAX_DELAY);

  //
  decltype(_acks) retval;
  auto const bytes_received{xMessageBufferReceive(out::rx_message_buffer.handle,
                                                  data(retval),
                                                  retval.max_size(),
                                                  portMAX_DELAY)};
  assert(bytes_received == retval.max_size());
  return retval;
}

/// \todo document
void Service::close() { _queue = {}; }

} // namespace mdu
