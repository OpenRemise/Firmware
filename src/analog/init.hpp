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

} // namespace analog