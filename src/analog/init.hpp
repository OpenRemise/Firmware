// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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

esp_err_t init(BaseType_t xCoreID);

}  // namespace analog