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

/// Deinitialize peripherals when suspending DCC task
///
/// \file   out/track/dcc/suspend.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "suspend.hpp"
#include <driver/gpio.h>
#include <driver/uart.h>
#include "../../suspend.hpp"

namespace out::track::dcc {

/// \todo document
/// For some reason the delay seems to be necessary when switching between op-
/// and serv-mode?
esp_err_t deinit_gpio() {
  ESP_ERROR_CHECK(gpio_set_level(enable_gpio_num, 0u));
  vTaskDelay(pdMS_TO_TICKS(20u));
  return gpio_set_level(right_force_low_gpio_num, 1u);
}

/// \todo document
esp_err_t deinit_bidi() {
  ESP_ERROR_CHECK(gpio_set_level(bidi_en_gpio_num, 0u));
  return uart_driver_delete(UART_NUM_1);
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

/// \todo document
esp_err_t deinit_rmt() {
  rmt_tx_event_callbacks_t cbs{};
  return rmt_tx_register_event_callbacks(channel, &cbs, NULL);
}

/// \todo document
esp_err_t deinit_encoder() {
  ESP_ERROR_CHECK(rmt_del_encoder(encoder));
  encoder = NULL;
  return ESP_OK;
}

/// \todo document
esp_err_t suspend() {
  ESP_ERROR_CHECK(deinit_gpio());
  ESP_ERROR_CHECK(deinit_bidi());
  ESP_ERROR_CHECK(deinit_alarm());
  ESP_ERROR_CHECK(deinit_rmt());
  ESP_ERROR_CHECK(deinit_encoder());
  return out::suspend();
}

}  // namespace out::track::dcc