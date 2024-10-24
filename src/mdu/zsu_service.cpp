// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "zsu_service.hpp"
#include <ulf/mdu_ein.hpp>
#include "log.h"
#include "utility.hpp"

namespace mdu {

using ::ulf::mdu_ein::ack, ::ulf::mdu_ein::nak;

/// \bug should this broadcast Z21 programming mode?
esp_err_t ZsuService::zsuSocket(http::Message& msg) {
  //
  if (auto expected{State::Suspended};
      msg.type != HTTPD_WS_TYPE_CLOSE &&
      state.compare_exchange_strong(expected, State::MDUZsu)) {
    LOGI_TASK_RESUME(task.handle);
    LOGI_TASK_RESUME(out::track::mdu::task.handle);
  }

  //
  if (state.load() == State::MDUZsu) {
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
      case HTTPD_WS_TYPE_BINARY: _acks = transmit(msg.payload); break;
      case HTTPD_WS_TYPE_CLOSE: LOGI("WebSocket closed"); return close();
      default:
        LOGE("WebSocket packet type neither binary nor close");
        _acks = {ack, nak};
        break;
    }

    httpd_ws_frame_t frame{
      .type = HTTPD_WS_TYPE_BINARY,
      .payload = data(_acks),
      .len = size(_acks),
    };
    httpd_ws_send_frame_async(msg.sock_fd, &frame);

    _queue.pop();
  }
}

/// \todo document
std::array<uint8_t, 2uz>
ZsuService::transmit(std::vector<uint8_t> const& payload) const {
  //
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(payload),
                     size(payload),
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
void ZsuService::close() {
  _queue = {};
  bug_led(false);
}

}  // namespace mdu
