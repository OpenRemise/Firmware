/// Cover /ota/ endpoint
///
/// \file   ota/service.cpp
/// \author Vincent Hamp
/// \date   13/06/2023

#pragma once

#include "service.hpp"
#include <esp_app_desc.h>
#include <esp_app_format.h>
#include "log.h"
#include "utility.hpp"

namespace ota {

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
      mode.compare_exchange_strong(expected, Mode::OTA))
    LOGI_TASK_RESUME(task.handle);

  //
  if (mode.load() == Mode::OTA) {
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
        LOGI("OTA WebSocket timeout");
        return close();
      }

    auto const& msg{_queue.front()};

    switch (msg.type) {
      case HTTPD_WS_TYPE_BINARY: _ack = write(msg.payload); break;
      case HTTPD_WS_TYPE_CLOSE:
        LOGI("OTA WebSocket closed");
        if (_handle) end();
        return close();
      default:
        LOGE("OTA WebSocket packet type neither binary nor close");
        _ack = nak;
        break;
    }

    // Send frame
    httpd_ws_frame_t ws_pkt{
      .type = HTTPD_WS_TYPE_BINARY,
      .payload = &_ack,
      .len = sizeof(_ack),
    };
    httpd_ws_send_frame_async(msg.handle, msg.fd, &ws_pkt);

    // We can't continue in case of error... so abort
    if (_ack == nak) return close();

    _queue.pop();
  }
}

/// TODO
uint8_t Service::write(std::vector<uint8_t> const& payload) {
  //
  if (!_partition) _partition = esp_ota_get_next_update_partition(NULL);

  //
  if (!_handle)
    if (auto const err{
          esp_ota_begin(_partition, OTA_WITH_SEQUENTIAL_WRITES, &_handle)}) {
      LOGE("OTA update failed %s", esp_err_to_name(err));
      return nak;
    }

  //
  if (auto const err{esp_ota_write(_handle, data(payload), size(payload))}) {
    LOGE("OTA update failed %s", esp_err_to_name(err));
    return nak;
  }

  return ack;
}

/// TODO
void Service::end() {
  auto err{esp_ota_end(_handle)};
  if (err == ESP_OK) err = esp_ota_set_boot_partition(_partition);
  if (err == ESP_OK) {
    LOGI("OTA update successful, restarting...");
    bug_led(false);
    esp_restart();
  }
  LOGE("OTA update failed %s", esp_err_to_name(err));
}

/// TODO
void Service::close() {
  _queue = {};
  _partition = NULL;
  if (_handle) esp_ota_abort(_handle);
  _handle = {};
  _ack = {};
  auto expected{Mode::OTA};
  assert(mode.compare_exchange_strong(expected, Mode::Suspended));
  bug_led(false);
}

}  // namespace ota