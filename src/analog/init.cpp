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

/// Initialize ADC
///
/// \file   analog/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <driver/gpio.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_freertos_hooks.h>
#include "adc_task_function.hpp"
#include "convert.hpp"
#include "log.h"
#include "temp_task_function.hpp"

namespace analog {

namespace {

/// \todo document
esp_err_t init_gpio() {
  static constexpr gpio_config_t io_conf{.pin_bit_mask = 1ull << ol_on_gpio_num,
                                         .mode = GPIO_MODE_OUTPUT,
                                         .pull_up_en = GPIO_PULLUP_DISABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE};
  ESP_ERROR_CHECK(gpio_config(&io_conf));
  return gpio_set_level(ol_on_gpio_num, 0u);
}

}  // namespace

/// \todo document
esp_err_t init(BaseType_t xCoreID) {
  voltages_queue.handle =
    xQueueCreate(voltages_queue.size, sizeof(VoltagesQueue::value_type));
  currents_queue.handle =
    xQueueCreate(currents_queue.size, sizeof(CurrentsQueue::value_type));
  temperature_queue.handle =
    xQueueCreate(temperature_queue.size, sizeof(TemperatureQueue::value_type));

  ESP_ERROR_CHECK(init_gpio());

  //
  static constexpr adc_cali_curve_fitting_config_t cali_config{
    .unit_id = ADC_UNIT_1,
    .atten = attenuation,
    .bitwidth = static_cast<adc_bitwidth_t>(SOC_ADC_DIGI_MAX_BITWIDTH),
  };
  static_assert(SOC_ADC_DIGI_MAX_BITWIDTH == ADC_BITWIDTH_12);
  ESP_ERROR_CHECK(
    adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));

  // Both of those values are sizes in bytes
  adc_continuous_handle_cfg_t adc_config = {
    .max_store_buf_size = conversion_frame_size * 2u,
    .conv_frame_size = conversion_frame_size,
  };
  ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc1_handle));

  //
  auto pattern{std::invoke([] {
    std::array<adc_digi_pattern_config_t, size(channels)> retval;
    for (auto i{0uz}; i < size(channels); ++i)
      retval[i] = {
        .atten = attenuation,
        .channel = channels[i],
        .unit = ADC_UNIT_1,
        .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
      };
    return retval;
  })};
  adc_continuous_config_t dig_cfg{
    .pattern_num = size(pattern),
    .adc_pattern = data(pattern),
    .sample_freq_hz = sample_freq_hz,
    .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
  };
  ESP_ERROR_CHECK(adc_continuous_config(adc1_handle, &dig_cfg));

  //
  static constexpr temperature_sensor_config_t temp_sensor_config =
    TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
  ESP_ERROR_CHECK(
    temperature_sensor_install(&temp_sensor_config, &temp_sensor));
  ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));

  //
  if (!xTaskCreatePinnedToCore(adc_task_function,
                               adc_task.name,
                               adc_task.stack_size,
                               NULL,
                               adc_task.priority,
                               &adc_task.handle,
                               xCoreID))
    assert(false);
  if (!xTaskCreatePinnedToCore(temp_task_function,
                               temp_task.name,
                               temp_task.stack_size,
                               NULL,
                               temp_task.priority,
                               &temp_task.handle,
                               xCoreID))
    assert(false);

  return ESP_OK;
}

}  // namespace analog