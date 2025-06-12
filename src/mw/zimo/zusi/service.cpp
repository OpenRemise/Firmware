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
#include <ulf/susiv2.hpp>
#include <ztl/utility.hpp>
#include "drv/led/bug.hpp"
#include "log.h"
#include "utility.hpp"

namespace mw::zimo::zusi {

using ::ulf::susiv2::ack, ::ulf::susiv2::nak;

/// \todo document
Service::Service() {
  task.function = ztl::make_trampoline(this, &Service::taskFunction);
}

/// \todo document
/// \bug should this broadcast Z21 programming mode?
esp_err_t Service::socket(intf::http::Message& msg) {
  //
  if (auto expected{State::Suspended};
      msg.type != HTTPD_WS_TYPE_CLOSE &&
      state.compare_exchange_strong(expected, State::ZUSI)) {
    _queue.push(std::move(msg));
    LOGI_TASKS_CREATE(task, drv::out::susi::zimo::zusi::task);
    return ESP_OK;
  }
  //
  else if (state.load() == State::ZUSI) {
    _queue.push(std::move(msg));
    return ESP_OK;
  }
  //
  else
    return ESP_FAIL;
}

/// \todo document
[[noreturn]] void Service::taskFunction(void*) {
  switch (state.load()) {
    case State::ZUSI: loop(); break;
    default: assert(false); break;
  }
  LOGI_TASK_DESTROY();
}

/// \todo document
void Service::loop() {
  drv::led::Bug const led_bug{};
  auto const timeout{http_receive_timeout2ms()};

  for (;;) {
    assert(_queue.size());
    auto const msg{std::move(_queue.front())};
    _queue.pop();

    switch (msg.type) {
      case HTTPD_WS_TYPE_BINARY: _resp = transmit(msg.payload); break;
      case HTTPD_WS_TYPE_CLOSE: LOGI("WebSocket closed"); return close();
      default:
        LOGE("WebSocket packet type neither binary nor close");
        _resp.front() = nak;
        _resp.resize(sizeof(nak));
        break;
    }

    if (auto const err{httpd_queue_work(new intf::http::Message{
          .sock_fd = msg.sock_fd,
          .type = HTTPD_WS_TYPE_BINARY,
          .payload = {cbegin(_resp), cend(_resp)},
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
::ulf::susiv2::Response
Service::transmit(std::vector<uint8_t> const& payload) const {
  //
  xMessageBufferSend(drv::out::tx_message_buffer.front_handle,
                     data(payload),
                     std::min(size(payload), ZUSI_MAX_PACKET_SIZE),
                     portMAX_DELAY);

  //
  decltype(_resp) retval;
  auto const bytes_received{
    xMessageBufferReceive(drv::out::rx_message_buffer.handle,
                          data(retval),
                          retval.max_size(),
                          portMAX_DELAY)};
  retval.resize(bytes_received);

  return retval;
}

/// \todo document
void Service::close() {
  _queue = {};

  // send exit command... just in case?
  // std::array<uint8_t, 10uz> exit_cmd{};
  // exit_cmd[5uz] = std::to_underlying(Command::Exit);
  // exit_cmd[6uz] = 0x55u;
  // exit_cmd[7uz] = 0xAAu;
  // exit_cmd[8uz] = 0xFFu;  // No reboot, no CV8 reset
  // exit_cmd[9uz] = crc8({cbegin(exit_cmd), 9uz});
  // xMessageBufferSend(out::tx_message_buffer.front_handle,
  //                    data(exit_cmd),
  //                    size(exit_cmd),
  //                    portMAX_DELAY);

  /// \todo wait for ZUSI task to suspend itself?

  // auto expected{State::ZUSI};
  // state.compare_exchange_strong(expected, State::Suspend);
}

} // namespace mw::zimo::zusi
