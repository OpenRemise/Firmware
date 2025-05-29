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

/// ESP-IDF application entry point
///
/// \file   app_main.cpp
/// \author Vincent Hamp
/// \date   26/12/2022

#include <driver/gpio.h>

/// ESP-IDF application entry point
extern "C" void app_main() {
  // Inputs and outputs
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << out::track::enable_gpio_num,
      .mode = GPIO_MODE_INPUT_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(out::track::enable_gpio_num, 0u));
  }

  // Outputs
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << led::bug_gpio_num |
                      1ull << out::track::p_gpio_num |
                      1ull << out::track::ilim0_gpio_num |
                      1ull << out::track::ilim1_gpio_num |
                      1ull << out::track::nsleep_gpio_num |
                      1ull << out::track::n_force_low_gpio_num |
                      1ull << out::track::dcc::bidi_en_gpio_num,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(out::track::p_gpio_num, 1u));
    ESP_ERROR_CHECK(gpio_set_level(out::track::n_force_low_gpio_num, 1u));
    ESP_ERROR_CHECK(gpio_set_level(out::track::dcc::bidi_en_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(out::track::nsleep_gpio_num, 1u));
  }

  // Wait for device to wake up
  // (otherwise nFAULT would immediately trigger an interrupt)
  vTaskDelay(pdMS_TO_TICKS(100u));

  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << out::track::nfault_gpio_num,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_NEGEDGE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
  }

  // Wait 10s
  vTaskDelay(pdMS_TO_TICKS(10000u));

  // Enable outputs
  ESP_ERROR_CHECK(gpio_set_level(out::track::enable_gpio_num, 1u));
  vTaskDelay(pdMS_TO_TICKS(20u));
  ESP_ERROR_CHECK(gpio_set_level(out::track::n_force_low_gpio_num, 0u));

  for (;;) {
    ESP_ERROR_CHECK(gpio_set_level(out::track::p_gpio_num, 1u));
    ESP_ERROR_CHECK(gpio_set_level(led::bug_gpio_num, 1u));
    vTaskDelay(pdMS_TO_TICKS(10000u));
    ESP_ERROR_CHECK(gpio_set_level(out::track::p_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(led::bug_gpio_num, 0u));
    vTaskDelay(pdMS_TO_TICKS(10000u));
  }
}
