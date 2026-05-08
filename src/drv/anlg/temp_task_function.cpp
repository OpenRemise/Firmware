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

/// ADC temperature task function
///
/// \file   drv/anlg/temp_task_function.cpp
/// \author Vincent Hamp
/// \date   05/07/2023

#include "init.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"

namespace drv::anlg {

extern std::atomic<uint8_t> nvs_short_circuit_time;

/// Temperature task function
///
/// Once started, the temperature task runs continuously. The internal
/// temperature sensor is read once per second and the result is converted into
/// degrees Celsius. The result is written to the corresponding \ref
/// temperature_queue "temperature" queue.
[[noreturn]] void temp_task_function(void*) {
  for (;;) {
    TemperatureQueue::value_type temp;
    ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &temp));
    xQueueOverwrite(temperature_queue.handle, &temp);
    vTaskDelay(pdMS_TO_TICKS(1000u));

    // Ugly workaround to update initial short circuit time from NVS. This
    // atomic is used in the `adc_task_function` but can't be updated there due
    // to timing constraints.
    nvs_short_circuit_time.store(
      mem::nvs::Settings{}.getCurrentShortCircuitTime());
  }
}

} // namespace drv::anlg
