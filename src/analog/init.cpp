// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize ADC
///
/// \file   analog/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <esp_adc/adc_cali_scheme.h>
#include <esp_freertos_hooks.h>
#include "adc_task_function.hpp"
#include "convert.hpp"
#include "log.h"
#include "temp_task_function.hpp"

namespace analog {

/// \todo document
esp_err_t init(BaseType_t xCoreID) {
  voltages_queue.handle =
    xQueueCreate(voltages_queue.size, sizeof(VoltagesQueue::value_type));
  currents_queue.handle =
    xQueueCreate(currents_queue.size, sizeof(CurrentsQueue::value_type));
  temperature_queue.handle =
    xQueueCreate(temperature_queue.size, sizeof(TemperatureQueue::value_type));

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