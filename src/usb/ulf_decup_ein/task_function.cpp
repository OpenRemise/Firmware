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

/// ULF_DECUP_EIN protocol task function
///
/// \file   usb/ulf_decup_ein/task_function.cpp
/// \author Vincent Hamp
/// \date   10/08/2024

#include "task_function.hpp"
#include <expected>
#include <ulf/decup_ein.hpp>
#include "../tx_task_function.hpp"
#include "log.h"
#include "utility.hpp"

namespace usb::ulf_decup_ein {

using namespace ulf::decup_ein;

namespace {

/// \todo document
class ZsuLoad : public ::ulf::decup_ein::rx::Base {
  uint8_t transmit(std::span<uint8_t const> bytes) final {
    uint8_t acks{};

    if (auto const timeout{http_receive_timeout2ms()};
        !xMessageBufferSend(out::tx_message_buffer.front_handle,
                            data(bytes),
                            size(bytes),
                            pdMS_TO_TICKS(timeout)))
      return acks;
    else
      xMessageBufferReceive(out::rx_message_buffer.handle,
                            &acks,
                            sizeof(acks),
                            pdMS_TO_TICKS(timeout));

    /// \todo remove
    // for (auto c : bytes) printf("%X ", c);
    // printf(" -> %d\n", acks);

    return acks;
  }
};

/// \todo document
void transmit_response(uint8_t byte) {
  xStreamBufferSend(
    tx_stream_buffer.handle, &byte, sizeof(byte), portMAX_DELAY);
}

/// Actual usb::decup_ein::task loop
void loop() {
  ZsuLoad zsu_load{};

  for (;;) {
    // Receive single character
    uint8_t byte;
    if (!xStreamBufferReceive(rx_stream_buffer.handle,
                              &byte,
                              sizeof(byte),
                              pdMS_TO_TICKS(http_receive_timeout2ms())))
      return;

    //
    if (auto const resp{zsu_load.receive(byte)}) transmit_response(*resp);
  }
}

} // namespace

/// \todo document
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    //
    if (auto expected{State::Suspended};
        state.compare_exchange_strong(expected, State::ULF_DECUP_EIN)) {
      transmit_ok();
      LOGI_TASK_RESUME(out::track::decup::task.handle);
      loop();
    }
    //
    else
      transmit_not_ok();

    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

} // namespace usb::ulf_decup_ein