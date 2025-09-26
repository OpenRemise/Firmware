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

/// WiFi LED
///
/// \file   drv/led/wifi.cpp
/// \author Vincent Hamp
/// \date   27/04/2025

#include "wifi.hpp"
#include <driver/ledc.h>
#include "mem/nvs/settings.hpp"

namespace drv::led {

  namespace {
    int blink_on;
    int blink_off;

    uint32_t calculate_duty() {
      mem::nvs::Settings nvs;
      auto const dc = nvs.getLedDutyCycleWiFi();
      return (dc * 256u) / 100u;
    }

    static TaskHandle_t blink_task_handle = nullptr;
    static void blink_task(void*) {
      uint32_t duty = calculate_duty();
      for (;;) {
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, wifi_channel, duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, wifi_channel));
        vTaskDelay(pdMS_TO_TICKS(blink_on));
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, wifi_channel, 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, wifi_channel));
        vTaskDelay(pdMS_TO_TICKS(blink_off));
      }
    }
  }  // namespace

  void wifi::on() {
    if (blink_task_handle) 
      vTaskSuspend(blink_task_handle);
    auto const duty = calculate_duty();
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, wifi_channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, wifi_channel));
  }

  void wifi::off() {
    if (blink_task_handle) 
      vTaskSuspend(blink_task_handle);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, wifi_channel, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, wifi_channel));
  }

  void wifi::blink(int on, int off) {
    blink_on = on;
    blink_off = off;
    
    if (blink_task_handle) {
      vTaskResume(blink_task_handle);
    } else {
      xTaskCreate(blink_task, "wifi_blink", 2048, nullptr, 5, &blink_task_handle);
    }
  }
} // namespace drv::led
