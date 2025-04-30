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

/// Initialize LED
///
/// \file   led/init.cpp
/// \author Vincent Hamp
/// \date   27/04/2025

#include "init.hpp"
#include <driver/ledc.h>

namespace led {

/// \todo document
esp_err_t init() {
  static constexpr ledc_timer_config_t timer{.speed_mode = LEDC_LOW_SPEED_MODE,
                                             .duty_resolution =
                                               LEDC_TIMER_8_BIT,
                                             .timer_num = LEDC_TIMER_0,
                                             .freq_hz = 4000,
                                             .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&timer));

  {
    static constexpr ledc_channel_config_t cfg{.gpio_num = bug_gpio_num,
                                               .speed_mode =
                                                 LEDC_LOW_SPEED_MODE,
                                               .channel = bug_channel,
                                               .intr_type = LEDC_INTR_DISABLE,
                                               .timer_sel = LEDC_TIMER_0,
                                               .duty = 0u,
                                               .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&cfg));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, bug_channel, 0u));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, bug_channel));
  }

  {
    static constexpr ledc_channel_config_t cfg{.gpio_num = wifi_gpio_num,
                                               .speed_mode =
                                                 LEDC_LOW_SPEED_MODE,
                                               .channel = wifi_channel,
                                               .intr_type = LEDC_INTR_DISABLE,
                                               .timer_sel = LEDC_TIMER_0,
                                               .duty = 0u,
                                               .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&cfg));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, wifi_channel, 0u));
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, wifi_channel);
  }
}

} // namespace led
