/// Initialize ADC
///
/// \file   analog/init.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <driver/temperature_sensor.h>
#include <esp_adc/adc_continuous.h>
#include <esp_err.h>
#include <array>

namespace analog {

inline adc_continuous_handle_t adc1_handle{};
inline temperature_sensor_handle_t temp_sensor{};

esp_err_t init();

}  // namespace analog