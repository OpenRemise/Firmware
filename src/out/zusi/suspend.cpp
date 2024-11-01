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

/// Deinitialize peripherals when suspending ZUSI task
///
/// \file   out/zusi/suspend.cpp
/// \author Vincent Hamp
/// \date   27/03/2023

#include "suspend.hpp"
#include <driver/gpio.h>
#include <hal/gpio_ll.h>
#include <algorithm>
#include "../suspend.hpp"
#include "init.hpp"

namespace out::zusi {

namespace {

/// \todo document
esp_err_t deinit_gpio() {
  ESP_ERROR_CHECK(gpio_set_direction(data_gpio_num, GPIO_MODE_OUTPUT));
  esp_rom_gpio_connect_out_signal(
    data_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  esp_rom_gpio_connect_out_signal(
    clock_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  ESP_ERROR_CHECK(gpio_set_level(data_gpio_num, 0u));
  ESP_ERROR_CHECK(gpio_set_level(clock_gpio_num, 0u));
  return gpio_set_level(enable_gpio_num, 0u);
}

/// \todo document
esp_err_t deinit_alarm() {
  gptimer_stop(gptimer);
  ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0ull));
  ESP_ERROR_CHECK(gptimer_disable(gptimer));
  gptimer_event_callbacks_t cbs{};
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
  return gptimer_set_alarm_action(gptimer, NULL);
}

}  // namespace

/// \todo document
esp_err_t suspend() {
  ESP_ERROR_CHECK(deinit_gpio());
  ESP_ERROR_CHECK(deinit_alarm());
  return out::suspend();
}

}  // namespace out::zusi