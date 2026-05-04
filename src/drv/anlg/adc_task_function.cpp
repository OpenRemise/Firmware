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
/// \file   drv/anlg/adc_task_function.cpp
/// \author Vincent Hamp
/// \date   05/07/2023

#include <driver/gpio.h>
#include <ztl/moving_average.hpp>
#include "drv/led/bug.hpp"
#include "init.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "mw/roco/z21/service.hpp"
#include "utility.hpp"

namespace drv::anlg {

namespace {

std::array<uint8_t, conversion_frame_size> conversion_frame;
ztl::moving_average<int32_t, smath::pow(2, 12)> filtered_current;

/// \todo document
BaseType_t xQueueSendOverwriteOldest(QueueHandle_t xQueue,
                                     void const* pvItemToQueue) {
  if (!xQueueSend(xQueue, pvItemToQueue, 0u)) {
    int16_t dummy;
    xQueueReceive(xQueue, &dummy, 0u);
    return xQueueSend(xQueue, pvItemToQueue, 0u);
  }
  return pdPASS;
}

/// Handles suspend/resume logic
///
/// Since the ADC task holds a mutex on the [continuous mode
/// driver](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/peripherals/adc_continuous.html#_CPPv427adc_continuous_handle_cfg_t),
/// the only way to stop and start the ADC is to send a task notification. This
/// is done using adc_task_notify_suspend() or adc_task_notify_resume(). The
/// conversion frame then starts again from the beginning and this method can be
/// used to synchronize the ADC.
void handle_suspend_resume_on_notify() {
  uint32_t notified_value{};
  xTaskNotifyWaitIndexed(default_notify_index, 0u, 0u, &notified_value, 0u);
  if (notified_value) {
    ESP_ERROR_CHECK(adc_continuous_stop(adc1_handle));
    ESP_ERROR_CHECK(adc_continuous_flush_pool(adc1_handle));
    xTaskNotifyWaitIndexed(
      default_notify_index, 0u, ULONG_MAX, &notified_value, portMAX_DELAY);
    assert(!notified_value);
    ESP_ERROR_CHECK(adc_continuous_start(adc1_handle));
  }
}

} // namespace

/// Suspend ADC task with task notification
void adc_task_notify_suspend() {
  xTaskNotifyIndexed(
    adc_task.handle, default_notify_index, 1u, eSetValueWithOverwrite);
}

/// Resume ADC task with task notification
void adc_task_notify_resume() {
  xTaskNotifyIndexed(
    adc_task.handle, default_notify_index, 0u, eSetValueWithOverwrite);
}

/// ADC task function
///
/// Once started, the ADC task runs continuously. It measures voltages and
/// currents at a frequency of \ref sample_freq_hz "83333Hz". A total of \ref
/// conversion_frame_samples "84" samples are recorded within one conversion
/// frame meaning one frame lasts approximately \ref conversion_frame_time
/// "1ms". All measurements are written to the corresponding \ref
/// vcc_voltages_queue "VCC voltages",
/// \ref supply_voltages_queue "supply voltages" or
/// \ref currents_queue "currents" queue.
///
/// If the measured currents indicate a short circuit, the \ref led::bug
/// "bug LED" is switched on, \ref state is set to \ref State::ShortCircuit
/// "short circuit" and a \ref page_mw_roco track short circuit message is
/// broadcast.
[[noreturn]] void adc_task_function(void*) {
  auto const initial_short_circuit_time{
    mem::nvs::Settings{}.getCurrentShortCircuitTime()};
  auto short_circuit_time{initial_short_circuit_time};

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

    // The following conversion and copy takes 359us
    size_t short_circuit_count{};
    for (auto i{0uz}; i < bytes_received; i += SOC_ADC_DIGI_RESULT_BYTES) {
      auto const output{
        std::bit_cast<adc_digi_output_data_t*>(&conversion_frame[i])};
      auto const data{static_cast<int16_t>(output->type2.data)};
      auto const chan{output->type2.channel};
      switch (chan) {
        case vcc_voltage_channel:
          xQueueSendOverwriteOldest(vcc_voltages_queue.handle, &data);
          break;
        case supply_voltage_channel:
          xQueueSendOverwriteOldest(supply_voltages_queue.handle, &data);
          break;
        case current_channel:
          xQueueSendOverwriteOldest(currents_queue.handle, &data);
          filtered_current += data;
          short_circuit_count += data == max_measurement;
          break;
        default: assert(false); break;
      }
    }
    auto const data{static_cast<int16_t>(filtered_current.value())};
    xQueueOverwrite(filtered_current_queue.handle, &data);

    // >90% of all current measurements indicate short circuit
    if (static constexpr auto ninety_pct{
          static_cast<size_t>(0.9 * conversion_frame_samples_per_channel)};
        state.load() != State::ShortCircuit &&         // Not ShortCircuit
        short_circuit_count > ninety_pct &&            // 90% max measurements
        !--short_circuit_time &&                       // Time until reaction
        gpio_get_level(out::track::enable_gpio_num)) { // Tracks enabled
      state.store(State::ShortCircuit);
      led::bug(true);
      mw::roco::z21::service->broadcastTrackShortCircuit();
    }
    // Clear count if no short circuit
    else
      short_circuit_time = initial_short_circuit_time;

    // Handle suspend/resume on notify
    handle_suspend_resume_on_notify();
  }
}

} // namespace drv::anlg
