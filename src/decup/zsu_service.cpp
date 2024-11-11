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

#include "zsu_service.hpp"
#include "log.h"
#include "utility.hpp"

namespace decup {

using ::ulf::decup_ein::ack, ::ulf::decup_ein::nak;

/// \bug should this broadcast Z21 programming mode?
esp_err_t ZsuService::zsuSocket(http::Message& msg) {
  //
  if (auto expected{State::Suspended};
      msg.type != HTTPD_WS_TYPE_CLOSE &&
      state.compare_exchange_strong(expected, State::DECUPZsu)) {
    LOGI_TASK_RESUME(task.handle);
    // LOGI_TASK_RESUME(out::track::decup::task.handle);
  }

  //
  if (state.load() == State::DECUPZsu) {
    _queue.push(std::move(msg));
    return ESP_OK;
  }
  //
  else
    return ESP_FAIL;
}

/// \todo document
void ZsuService::loop() {
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
uint8_t ZsuService::transmit(std::span<uint8_t const> bytes) {
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

void ZsuService::done() {
  /// \todo call close and stuff here? this also needs to stop the actual DECUP
  /// task, just like the USB version
}

/// \todo document
void ZsuService::close() {
  _queue = {};
  bug_led(false);
}

}  // namespace decup
