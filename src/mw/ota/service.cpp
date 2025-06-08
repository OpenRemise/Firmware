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

/// Cover /ota/ endpoint
///
/// \file   ota/service.cpp
/// \author Vincent Hamp
/// \date   13/06/2023

#pragma once

#include "service.hpp"
#include <esp_app_desc.h>
#include <esp_app_format.h>
#include <ztl/utility.hpp>
#include "drv/led/bug.hpp"
#include "log.h"
#include "utility.hpp"

namespace ota {

/// \todo document
/// \bug should this broadcast Z21 programming mode?
Service::Service() {
  task.create(ztl::make_trampoline(this, &Service::taskFunction));
}

/// \todo document
Service::~Service() { task.destroy(); }

/// \todo document
esp_err_t Service::socket(http::Message& msg) {
  //
  if (auto expected{State::Suspended};
      msg.type != HTTPD_WS_TYPE_CLOSE &&
      state.compare_exchange_strong(expected, State::OTA)) {
    _queue.push(std::move(msg));
    LOGI_TASK_RESUME(task);
    return ESP_OK;
  }
  //
  else if (state.load() == State::OTA) {
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
      case State::OTA: loop(); break;
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
      case HTTPD_WS_TYPE_BINARY: _ack = write(msg.payload); break;
      case HTTPD_WS_TYPE_CLOSE:
        LOGI("WebSocket closed");
        if (_handle) end();
        return close();
      default:
        LOGE("WebSocket packet type neither binary nor close");
        _ack = nak;
        break;
    }

    if (auto const err{httpd_queue_work(new http::Message{
          .sock_fd = msg.sock_fd,
          .type = HTTPD_WS_TYPE_BINARY,
          .payload = {_ack},
        })}) {
      LOGE("httpd_queue_work failed %s", esp_err_to_name(err));
      return close();
    }

    // We can't continue in case of error... so abort
    if (_ack == nak) return close();

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
uint8_t Service::write(std::vector<uint8_t> const& payload) {
  //
  if (!_partition) _partition = esp_ota_get_next_update_partition(NULL);

  //
  if (!_handle)
    if (auto const err{
          esp_ota_begin(_partition, OTA_WITH_SEQUENTIAL_WRITES, &_handle)}) {
      LOGE("Update failed %s", esp_err_to_name(err));
      return nak;
    }

  //
  if (auto const err{esp_ota_write(_handle, data(payload), size(payload))}) {
    LOGE("Update failed %s", esp_err_to_name(err));
    return nak;
  }

  return ack;
}

/// \todo document
void Service::end() {
  auto err{esp_ota_end(_handle)};
  if (err == ESP_OK) err = esp_ota_set_boot_partition(_partition);
  if (err == ESP_OK) {
    LOGI("Update successful, restarting...");
    esp_restart();
  }
  LOGE("Update failed %s", esp_err_to_name(err));
}

/// \todo document
void Service::close() {
  _queue = {};
  _partition = NULL;
  if (_handle) esp_ota_abort(_handle);
  _handle = {};
  _ack = {};
  if (auto expected{State::OTA};
      !state.compare_exchange_strong(expected, State::Suspended))
    assert(false);
}

} // namespace ota
