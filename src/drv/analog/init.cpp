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

/// Initialize ADC
///
/// \file   drv/analog/init.cpp
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

namespace drv::analog {

namespace {

/// Initialize analog GPIO
///
/// Initializes the higher resolution current sense switching of the
/// TPS281C100x. This feature is currently not used.
///
/// \retval ESP_OK  Success
esp_err_t init_gpio() {
  static constexpr gpio_config_t io_conf{.pin_bit_mask = 1ull << ol_on_gpio_num,
                                         .mode = GPIO_MODE_OUTPUT,
                                         .pull_up_en = GPIO_PULLUP_DISABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE};
  ESP_ERROR_CHECK(gpio_config(&io_conf));
  return gpio_set_level(ol_on_gpio_num, 0u);
}

} // namespace

/// Initialize analog
///
/// Initialization takes place in init(). This function performs the following
/// operations:
/// - Creates queues for raw \ref voltages_queue "voltage" and \ref
///   currents_queue "current" vales as well as \ref temperature_queue
///   "temperatures" in Si units
/// - Initializes the ADC in [continuous
///   mode](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/peripherals/adc_continuous.html)
///   and applies a curve fitting calibration
/// - Initializes the internal temperature sensor
/// - Creates an ADC and temperature task
esp_err_t init() {
  voltages_queue.handle =
    xQueueCreate(voltages_queue.size, sizeof(VoltagesQueue::value_type));
  currents_queue.handle =
    xQueueCreate(currents_queue.size, sizeof(CurrentsQueue::value_type));
  temperature_queue.handle =
    xQueueCreate(temperature_queue.size, sizeof(TemperatureQueue::value_type));

  ESP_ERROR_CHECK(init_gpio());

  // Curve fitting calibration
  static constexpr adc_cali_curve_fitting_config_t cali_config{
    .unit_id = ADC_UNIT_1,
    .atten = attenuation,
    .bitwidth = static_cast<adc_bitwidth_t>(SOC_ADC_DIGI_MAX_BITWIDTH),
  };
  static_assert(SOC_ADC_DIGI_MAX_BITWIDTH == ADC_BITWIDTH_12);
  ESP_ERROR_CHECK(
    adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));

  // Both of those values are sizes in bytes
  static constexpr adc_continuous_handle_cfg_t adc_config{
    .max_store_buf_size = conversion_frame_size * 2u,
    .conv_frame_size = conversion_frame_size,
    .flags =
      {
        .flush_pool = true,
      },
  };
  ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc1_handle));

  // Create same pattern for both channels
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

  // Initialize internal temperature sensor
  static constexpr temperature_sensor_config_t temp_sensor_config =
    TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
  ESP_ERROR_CHECK(
    temperature_sensor_install(&temp_sensor_config, &temp_sensor));
  ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));

  // Create ADC and temp tasks
  adc_task.create(adc_task_function);
  temp_task.create(temp_task_function);

  return ESP_OK;
}

} // namespace drv::analog
