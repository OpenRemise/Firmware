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

/// ULF_DECUP_EIN task function
///
/// \file   ulf/decup_ein/task_function.cpp
/// \author Vincent Hamp
/// \date   04/05/2025

#include "task_function.hpp"
#include <ulf/decup_ein.hpp>
#include "log.h"
#include "usb/tx_task_function.hpp"
#include "utility.hpp"

namespace ulf::decup_ein {

using namespace ulf::decup_ein;

namespace {

/// \todo document
class Decup : public ::ulf::decup_ein::rx::Base {
  /// \todo document
  uint8_t transmit(std::span<uint8_t const> bytes) final {
    uint8_t acks{};
    if (!xMessageBufferSend(out::tx_message_buffer.front_handle,
                            data(bytes),
                            size(bytes),
                            pdMS_TO_TICKS(task.timeout)))
      return acks;
    else
      xMessageBufferReceive(out::rx_message_buffer.handle,
                            &acks,
                            sizeof(acks),
                            pdMS_TO_TICKS(task.timeout));
    return acks;
  }
};

/// \todo document
void transmit_response(uint8_t byte) {
  xStreamBufferSend(usb::tx_stream_buffer.handle,
                    &byte,
                    sizeof(byte),
                    pdMS_TO_TICKS(usb::tx_task.timeout));
}

/// Actual usb::decup_ein::task loop
void loop() {
  Decup decup{};

  // Wait for RTS on
  auto then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};
  while (!usb::rts && xTaskGetTickCount() < then)
    vTaskDelay(pdMS_TO_TICKS(100u));
  while (!xStreamBufferReset(usb::rx_stream_buffer.handle)) {
    LOGW("Can't reset usb::rx_stream_buffer");
    vTaskDelay(pdMS_TO_TICKS(20u));
  }

  // While RTS
  while (usb::rts && xTaskGetTickCount() < then) {
    // Receive single character
    uint8_t byte;
    if (!xStreamBufferReceive(usb::rx_stream_buffer.handle,
                              &byte,
                              sizeof(byte),
                              pdMS_TO_TICKS(100u)))
      continue;

    // Reset timeout
    then = xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout);

    // Transmit optional response
    if (auto const resp{decup.receive(byte)}) transmit_response(*resp);
  }

  // Suspend out::track::decup (if not already done so itself)
  if (auto expected{State::ULF_DECUP_EIN};
      state.compare_exchange_strong(expected, State::Suspend))
    ;
}

} // namespace

/// \todo document
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    //
    if (auto expected{State::Suspended};
        state.compare_exchange_strong(expected, State::ULF_DECUP_EIN)) {
      usb::transmit_ok();
      LOGI_TASK_RESUME(out::track::decup::task.handle);
      loop();
    }
    //
    else
      usb::transmit_not_ok();

    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

} // namespace ulf::decup_ein
