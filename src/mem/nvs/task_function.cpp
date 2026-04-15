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

/// NVS task function
///
/// \file   mem/nvs/task_function.cpp
/// \author Vincent Hamp
/// \date   06/12/2024

#include "task_function.hpp"
#include <driver/gpio.h>
#include "../../utility.hpp"
#include "drv/led/bug.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"

namespace mem::nvs {

namespace {

/// \todo document
void reset_sta_settings() {
  mem::nvs::Settings nvs;
  nvs.setStationmDNS("remise");
  nvs.setStationSSID("");
  nvs.setStationPassword("");
  nvs.setStationAlternativeSSID("");
  nvs.setStationAlternativePassword("");
  nvs.setStationIP("");
  nvs.setStationNetmask("");
  nvs.setStationGateway("");
}

} // namespace

/// \todo document
[[noreturn]] void task_function(void*) {
  size_t seconds{};

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000u));
    seconds = gpio_get_level(boot_gpio_num) ? 0uz : seconds + 1uz;
    if (seconds < 5uz) continue;
    drv::led::bug(true);
    reset_sta_settings();
    esp_delayed_restart();
    LOGI_TASK_DESTROY();
  }
}

} // namespace mem::nvs
