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

/// Deinitialize peripherals when suspending MDU task
///
/// \file   drv/out/track/zimo/mdu/suspend.cpp
/// \author Vincent Hamp
/// \date   10/04/2024

#include "suspend.hpp"
#include <driver/gpio.h>
#include "../../../suspend.hpp"

namespace drv::out::track::zimo::mdu {

namespace {

/// \todo document
esp_err_t deinit_gpio() {
  ESP_ERROR_CHECK(gpio_set_level(enable_gpio_num, 0u));
  vTaskDelay(pdMS_TO_TICKS(20u));
  ESP_ERROR_CHECK(gpio_set_level(n_force_low_gpio_num, 1u));
  return gpio_isr_handler_remove(ack_gpio_num);
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

} // namespace

/// \todo document
esp_err_t deinit_encoder() {
  ESP_ERROR_CHECK(rmt_del_encoder(encoder));
  encoder = NULL;
  return ESP_OK;
}

/// \todo document
esp_err_t suspend() {
  ESP_ERROR_CHECK(deinit_gpio());
  ESP_ERROR_CHECK(deinit_alarm());
  ESP_ERROR_CHECK(deinit_encoder());
  return out::suspend();
}

} // namespace drv::out::track::zimo::mdu
