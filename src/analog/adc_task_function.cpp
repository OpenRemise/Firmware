/// ADC voltage and current task function
///
/// \file   analog/adc_task_function.cpp
/// \author Vincent Hamp
/// \date   05/07/2023

#include "init.hpp"
#include "log.h"

namespace analog {

///
void adc_task_function(void*) {
  std::array<uint8_t, conversion_frame_size> conversion_frame{};

  // Start and stop must be called from the same task because the handle uses a
  // FreeRTOS mutex for internal locking
  ESP_ERROR_CHECK(adc_continuous_start(adc1_handle));

  for (;;) {
    uint32_t bytes_received;
    if (auto const err{adc_continuous_read(adc1_handle,
                                           data(conversion_frame),
                                           size(conversion_frame),
                                           &bytes_received,
                                           adc_task.timeout)}) {
      LOGE("%s", esp_err_to_name(err));
      continue;
    } else if (bytes_received != size(conversion_frame)) {
      LOGE("conversion length not equal buffer size");
      continue;
    }

    // The following conversion and copy takes 20.4µs
    VoltagesQueue::value_type voltages;
    CurrentsQueue::value_type currents;
    auto voltages_it{begin(voltages)};
    auto currents_it{begin(currents)};
    for (auto i{0uz}; i < bytes_received; i += SOC_ADC_DIGI_RESULT_BYTES) {
      auto const output{
        std::bit_cast<adc_digi_output_data_t*>(&conversion_frame[i])};
      auto const chan{output->type2.channel};
      auto const data{output->type2.data};
      if (chan == voltage_channel)
        *voltages_it++ = static_cast<VoltageMeasurement>(data);
      else if (chan == current_channel)
        *currents_it++ = static_cast<CurrentMeasurement>(data);
    }
    xQueueOverwrite(voltages_queue.handle, &voltages);
    xQueueOverwrite(currents_queue.handle, &currents);
  }

  // Start and stop must be called from the same task because the handle uses a
  // FreeRTOS mutex for internal locking
  ESP_ERROR_CHECK(adc_continuous_stop(adc1_handle));
}

}  // namespace analog