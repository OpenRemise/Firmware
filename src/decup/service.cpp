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

#include "service.hpp"
#include <zusi/zusi.hpp>
#include "log.h"
#include "utility.hpp"

namespace decup {

using ::ulf::decup_ein::ack, ::ulf::decup_ein::nak;

/// \todo document
Service::Service(BaseType_t xCoreID) {
  if (!xTaskCreatePinnedToCore(make_tramp(this, &Service::taskFunction),
                               task.name,
                               task.stack_size,
                               NULL,
                               task.priority,
                               &task.handle,
                               xCoreID))
    assert(false);
}

/// \todo document
Service::~Service() {
  if (task.handle) vTaskDelete(task.handle);
}

/// \bug should this broadcast Z21 programming mode?
esp_err_t Service::zppSocket(http::Message& msg) {
  return socket(msg, State::DECUPZpp);
}

/// \bug should this broadcast Z21 programming mode?
esp_err_t Service::zsuSocket(http::Message& msg) {
  return socket(msg, State::DECUPZsu);
}

/// \bug should this broadcast Z21 programming mode?
esp_err_t Service::socket(http::Message& msg, State decup_state) {
  //
  if (auto expected{State::Suspended};
      msg.type != HTTPD_WS_TYPE_CLOSE &&
      state.compare_exchange_strong(expected, decup_state)) {
    LOGI_TASK_RESUME(task.handle);
    // LOGI_TASK_RESUME(out::track::decup::task.handle);
  }

  //
  if (state.load() == decup_state) {
    _queue.push(std::move(msg));
    return ESP_OK;
  }
  //
  else
    return ESP_FAIL;
}

/// \todo document
void Service::taskFunction(void*) {
  for (;;) switch (state.load()) {
      case State::DECUPZpp: [[fallthrough]];
      case State::DECUPZsu:
        loop();
        vTaskDelay(pdMS_TO_TICKS(100u));
        break;
      default: LOGI_TASK_SUSPEND(task.handle); break;
    }
}

/// \todo document
void Service::loop() {
  bug_led(true);

  auto const timeout{http_receive_timeout2ms()};

  for (;;) {
    TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(timeout)};
    while (empty(_queue))
      if (xTaskGetTickCount() >= then) {
        LOGI("WebSocket timeout");
        return close();
      }

    auto const& msg{_queue.front()};

    switch (msg.type) {
      case HTTPD_WS_TYPE_BINARY:
        std::ranges::for_each(msg.payload,
                              [&](uint8_t byte) { _ack = receive(byte); });
        break;
      case HTTPD_WS_TYPE_CLOSE: LOGI("WebSocket closed"); return close();
      default:
        LOGE("WebSocket packet type neither binary nor close");
        _ack = nak;
        break;
    }

    httpd_ws_frame_t frame{
      .type = HTTPD_WS_TYPE_BINARY,
      .payload = _ack ? std::addressof(*_ack) : NULL,
      .len = _ack ? sizeof(*_ack) : 0uz,
    };
    httpd_ws_send_frame_async(msg.sock_fd, &frame);

    _queue.pop();
  }
}

/// \todo document
uint8_t Service::transmit(std::span<uint8_t const> bytes) {
  /// \todo remove once DECUP hardware issues are fixed
  /*
  //
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(bytes),
                     size(bytes),
                     portMAX_DELAY);

  //
  uint8_t retval;
  auto const bytes_received{xMessageBufferReceive(
    out::rx_message_buffer.handle, &retval, sizeof(retval), portMAX_DELAY)};
  return retval;
  */

  // Print incoming
  for (auto c : bytes) printf("%X ", c);

  /// \todo remove once DECUP hardware issues are fixed
  uint8_t acks;
  if (size(bytes) == 1uz) {
    // MX645 startbyte
    if (bytes[0uz] == 221u) acks = 2uz;
    else acks = 1uz;
  }
  // all other packets
  else
    acks = 2uz;

  printf(" -> %d\n", acks);
  return acks;
}

void Service::done() {
  /// \todo call close and stuff here? this also needs to stop the actual DECUP
  /// task, just like the USB version
}

/// \todo document
void Service::close() {
  _queue = {};
  bug_led(false);

  /// \todo remove once DECUP hardware issues are fixed
  state.store(State::Suspended);
}

}  // namespace decup