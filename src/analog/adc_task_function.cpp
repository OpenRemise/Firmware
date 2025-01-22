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

/// ADC voltage and current task function
///
/// \file   analog/adc_task_function.cpp
/// \author Vincent Hamp
/// \date   05/07/2023

#include <driver/gpio.h>
#include "init.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "utility.hpp"
#include "z21/service.hpp"

namespace analog {

namespace {

/// Convert the NVS setting "cur_sc_time" to a counter value
///
/// For convenience, the \ref mem::nvs::Settings::getCurrentShortCircuitTime()
/// "short circuit detection time" is stored in milliseconds. In order for this
/// setting to be used by the ADC task, it must be converted to a counter value.
///
/// \return Short circuit counter value for ADC task function
auto get_short_circuit_count() {
  return mem::nvs::Settings{}.getCurrentShortCircuitTime() /
         conversion_frame_time;
}

} // namespace

/// ADC task function
///
/// Once started, the ADC task runs continuously. It measures voltages and
/// currents at a frequency of \ref sample_freq_hz "8kHz". A total of \ref
/// conversion_frame_samples "160" samples are recorded within one conversion
/// frame meaning one frame lasts exactly \ref conversion_frame_time "20ms". All
/// measurements are written to the corresponding \ref voltages_queue "voltages"
/// or \ref currents_queue "currents" queue.
///
/// If the measured currents indicate a short circuit, the \ref bug_led
/// "bug LED" is switched, \ref state is set to \ref State::ShortCircuit
/// "short circuit" and a \ref page_z21 track short circuit message is
/// broadcast.
void adc_task_function(void*) {
  std::array<uint8_t, conversion_frame_size> conversion_frame{};
  auto short_circuit_count{get_short_circuit_count()};

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
      LOGE("adc_continuous_read failed %s", esp_err_to_name(err));
      continue;
    } else if (bytes_received != size(conversion_frame)) {
      LOGE("Conversion length not equal buffer size");
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

    // >90% of all current measurements indicate short circuit
    if (static constexpr auto ninty_pct{
          static_cast<size_t>(0.9 * size(currents))};
        state.load() != State::ShortCircuit &&
        std::ranges::count(currents, max_measurement) > ninty_pct &&
        !--short_circuit_count) {
      state.store(State::ShortCircuit);
      bug_led(true);
      z21::service->broadcastTrackShortCircuit();
    }
    // Clear count if no short circuit
    else
      short_circuit_count = get_short_circuit_count();
  }

  // Start and stop must be called from the same task because the handle uses a
  // FreeRTOS mutex for internal locking
  ESP_ERROR_CHECK(adc_continuous_stop(adc1_handle));
}

} // namespace analog