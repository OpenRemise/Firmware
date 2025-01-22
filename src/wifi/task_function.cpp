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

/// WiFi task function
///
/// \file   wifi/task_function.cpp
/// \author Vincent Hamp
/// \date   06/12/2024

#include "task_function.hpp"
#include <driver/gpio.h>
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "utility.hpp"

namespace wifi {

namespace {

/// \todo document
void reset_sta_settings() {
  mem::nvs::Settings settings;
  settings.setStationmDNS("remise");
  settings.setStationSSID("");
  settings.setStationPassword("");
}

} // namespace

/// \todo document
void task_function(void*) {
  size_t seconds{};

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000u));
    seconds = gpio_get_level(boot_gpio_num) ? 0uz : seconds + 1uz;
    if (seconds < 5uz) continue;
    bug_led(true);
    reset_sta_settings();
    esp_delayed_restart();
    vTaskDelete(NULL);
  }
}

} // namespace wifi