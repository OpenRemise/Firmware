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
#include <zusi/zusi.hpp>
#include "bug_led.hpp"
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
    _queue.push(std::move(msg));
    LOGI_TASK_RESUME(task.handle);
    LOGI_TASK_RESUME(out::track::decup::task.handle);
    return ESP_OK;
  }
  //
  else if (state.load() == decup_state) {
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
    LOGI_TASK_SUSPEND(task.handle);
    switch (state.load()) {
      case State::DECUPZpp: [[fallthrough]];
      case State::DECUPZsu: loop(); break;
      default: assert(false); break;
    }
  }
}

/// \todo document
void Service::loop() {
  BugLed const bug_led{true};

  for (;;) {
    assert(_queue.size());
    auto const msg{std::move(_queue.front())};
    _queue.pop();

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

    // Send frame
    httpd_ws_frame_t frame{
      .type = HTTPD_WS_TYPE_BINARY,
      .payload = _ack ? std::addressof(*_ack) : NULL,
      .len = _ack ? sizeof(*_ack) : 0uz,
    };
    if (auto const err{httpd_ws_send_frame_async(msg.sock_fd, &frame)}) {
      LOGE("httpd_ws_send_frame_async failed %s", esp_err_to_name(err));
      return close();
    }

    TickType_t const then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};
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
uint8_t Service::transmit(std::span<uint8_t const> bytes) {
  uint8_t acks{};
  if (!xMessageBufferSend(out::tx_message_buffer.front_handle,
                          data(bytes),
                          std::min(size(bytes), DECUP_MAX_PACKET_SIZE),
                          portMAX_DELAY))
    return acks;
  else
    xMessageBufferReceive(
      out::rx_message_buffer.handle, &acks, sizeof(acks), portMAX_DELAY);
  return acks;
}

/// \todo document
void Service::close() {
  _queue = {};
  state.store(State::Suspend);
}

} // namespace decup