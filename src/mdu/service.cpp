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
#include <ulf/mdu_ein.hpp>
#include "log.h"
#include "utility.hpp"

namespace mdu {

using ::ulf::mdu_ein::ack, ::ulf::mdu_ein::nak;

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
    LOGI_TASK_RESUME(task.handle);
    LOGI_TASK_RESUME(out::track::mdu::task.handle);
  }

  //
  if (state.load() == mdu_state) {
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
      case State::MDUZpp: [[fallthrough]];
      case State::MDUZsu:
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
Service::transmit(std::vector<uint8_t> const& payload) const {
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
void Service::close() {
  _queue = {};
  bug_led(false);
}

}  // namespace mdu