/// ADC temperature task function
///
/// \file   analog/temp_task_function.cpp
/// \author Vincent Hamp
/// \date   05/07/2023

#include "init.hpp"
#include "log.h"

namespace analog {

///
void temp_task_function(void*) {
  for (;;) {
    TemperatureQueue::value_type temp;
    ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &temp));
    xQueueOverwrite(temperature_queue.handle, &temp);
    vTaskDelay(pdMS_TO_TICKS(1000u));
  }
}

}  // namespace analog