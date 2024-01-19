#include "firmware_service.hpp"
#include "log.h"
#include "utility.hpp"

namespace mdu {

esp_err_t FirmwareService::firmwareSocket(http::Message& msg) {
  //
  if (auto expected{Mode::Suspended};
      msg.type != HTTPD_WS_TYPE_CLOSE &&
      mode.compare_exchange_strong(expected, Mode::MDUFirmware)) {
    LOGI_TASK_RESUME(task.handle);
    LOGI_TASK_RESUME(out::track::mdu::task.handle);
  }

  //
  if (mode.load() == Mode::MDUFirmware) {
    _queue.push(std::move(msg));
    return ESP_OK;
  }
  //
  else
    return ESP_FAIL;
}

/// TODO
void FirmwareService::loop() {
  bug_led(true);

  auto const timeout{get_http_receive_timeout()};

  for (;;) {
    TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(timeout)};
    while (empty(_queue))
      if (xTaskGetTickCount() >= then) {
        LOGI("MDU WebSocket timeout");
        return close();
      }

    auto const& msg{_queue.front()};

    switch (msg.type) {
      case HTTPD_WS_TYPE_BINARY: _acks = transmit(msg.payload); break;
      case HTTPD_WS_TYPE_CLOSE: LOGI("MDU WebSocket closed"); return close();
      default:
        LOGE("MDU WebSocket packet type neither binary nor close");
        _acks = {ack, nak};
        break;
    }

    httpd_ws_frame_t ws_pkt{
      .type = HTTPD_WS_TYPE_BINARY,
      .payload = data(_acks),
      .len = size(_acks),
    };
    httpd_ws_send_frame_async(msg.handle, msg.fd, &ws_pkt);

    _queue.pop();
  }
}

/// TODO
std::array<uint8_t, 2uz>
FirmwareService::transmit(std::vector<uint8_t> const& payload) const {
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

/// TODO
void FirmwareService::close() {
  _queue = {};
  bug_led(false);
}

}  // namespace mdu
