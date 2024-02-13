#include "service.hpp"
#include "log.h"
#include "utility.hpp"

namespace zusi {

/// TODO
Service::Service() {
  assert(xTaskCreatePinnedToCore(make_tramp(this, &Service::taskFunction),
                                 task.name,
                                 task.stack_depth,
                                 NULL,
                                 task.priority,
                                 &task.handle,
                                 1));
}

/// TODO
Service::~Service() {
  if (task.handle) vTaskDelete(task.handle);
}

/// TODO
esp_err_t Service::socket(http::Message& msg) {
  //
  if (auto expected{Mode::Suspended};
      msg.type != HTTPD_WS_TYPE_CLOSE &&
      mode.compare_exchange_strong(expected, Mode::ZUSI)) {
    LOGI_TASK_RESUME(task.handle);
    LOGI_TASK_RESUME(out::zusi::task.handle);
  }

  //
  if (mode.load() == Mode::ZUSI) {
    _queue.push(std::move(msg));
    return ESP_OK;
  }
  //
  else
    return ESP_FAIL;
}

/// TODO
void Service::taskFunction(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);
    loop();
  }
}

/// TODO
void Service::loop() {
  bug_led(true);

  auto const timeout{get_http_receive_timeout()};

  for (;;) {
    TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(timeout)};
    while (empty(_queue))
      if (xTaskGetTickCount() >= then) {
        LOGI("ZUSI WebSocket timeout");
        return close();
      }

    auto const& msg{_queue.front()};

    switch (msg.type) {
      case HTTPD_WS_TYPE_BINARY: _data = transmit(msg.payload); break;
      case HTTPD_WS_TYPE_CLOSE: LOGI("ZUSI WebSocket closed"); return close();
      default:
        LOGE("ZUSI WebSocket packet type neither binary nor close");
        _data.front() = nak;
        _data.resize(sizeof(nak));
        break;
    }

    //
    httpd_ws_frame_t ws_pkt{
      .type = HTTPD_WS_TYPE_BINARY,
      .payload = data(_data),
      .len = size(_data),
    };
    httpd_ws_send_frame_async(msg.handle, msg.fd, &ws_pkt);

    _queue.pop();
  }
}

/// TODO
ztl::inplace_vector<uint8_t, 8uz - 1uz>
Service::transmit(std::vector<uint8_t> const& payload) const {
  //
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(payload),
                     size(payload),
                     portMAX_DELAY);

  //
  decltype(_data) retval;
  auto const bytes_received{xMessageBufferReceive(out::rx_message_buffer.handle,
                                                  data(retval),
                                                  retval.max_size(),
                                                  portMAX_DELAY)};
  retval.resize(bytes_received);
  return retval;
}

/// TODO
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

  // TODO wait for ZUSI task to suspend itself?

  // auto expected{Mode::ZUSI};
  // assert(mode.compare_exchange_strong(expected, Mode::Blocked));
  bug_led(false);
}

}  // namespace zusi