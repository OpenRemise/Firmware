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

/// DRV8328 test
///
/// \file   drv8328_test.cpp
/// \author Vincent Hamp
/// \date   04/02/2026

#include <driver/gpio.h>

void drv8328_test() {
  // Inputs and outputs
  {
    static constexpr gpio_config_t gpio_cfg{
      .pin_bit_mask = 1ull << drv::out::track::enable_gpio_num,
      .mode = GPIO_MODE_INPUT_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&gpio_cfg));
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::enable_gpio_num, 0u));
  }

  // Outputs
  {
    static constexpr gpio_config_t gpio_cfg{
      .pin_bit_mask = 1ull << drv::led::bug_gpio_num |
                      1ull << drv::out::track::p_gpio_num |
                      1ull << drv::out::track::ilim0_gpio_num |
                      1ull << drv::out::track::ilim1_gpio_num |
                      1ull << drv::out::track::nsleep_gpio_num |
                      1ull << drv::out::track::n_force_low_gpio_num |
                      1ull << drv::out::track::dcc::bidi_en_gpio_num,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&gpio_cfg));
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::p_gpio_num, 1u));
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::ilim1_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::ilim0_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::n_force_low_gpio_num, 1u));
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::dcc::bidi_en_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::nsleep_gpio_num, 1u));
  }

  // Wait for device to wake up
  // (otherwise nFAULT would immediately trigger an interrupt)
  vTaskDelay(pdMS_TO_TICKS(100u));

  {
    static constexpr gpio_config_t gpio_cfg{
      .pin_bit_mask = 1ull << drv::out::track::nfault_gpio_num,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_NEGEDGE};
    ESP_ERROR_CHECK(gpio_config(&gpio_cfg));
  }

  // Wait 10s
  vTaskDelay(pdMS_TO_TICKS(10000u));

  // Enable outputs
  ESP_ERROR_CHECK(gpio_set_level(drv::out::track::enable_gpio_num, 1u));
  vTaskDelay(pdMS_TO_TICKS(20u));
  ESP_ERROR_CHECK(gpio_set_level(drv::out::track::n_force_low_gpio_num, 0u));

  static constexpr auto us{100'000u};
  for (;;) {
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::p_gpio_num, 1u));
    ESP_ERROR_CHECK(gpio_set_level(drv::led::bug_gpio_num, 1u));
    for (auto const then{esp_timer_get_time() + us};
         esp_timer_get_time() < then;);
    ESP_ERROR_CHECK(gpio_set_level(drv::out::track::p_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(drv::led::bug_gpio_num, 0u));
    for (auto const then{esp_timer_get_time() + us};
         esp_timer_get_time() < then;);
  }
}
