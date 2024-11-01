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

/// DECUP_EIN protocol task function
///
/// \file   usb/decup_ein/task_function.cpp
/// \author Vincent Hamp
/// \date   10/08/2024

#include "task_function.hpp"
#include <expected>
#include <ulf/decup_ein.hpp>
#include "../tx_task_function.hpp"
#include "log.h"
#include "utility.hpp"

namespace usb::decup_ein {

using namespace ulf::decup_ein;

namespace {

class ZsuLoad : public ulf::decup_ein::rx::Base {
  uint8_t transmit(std::span<uint8_t const> bytes) final {
    /**
    \todo remove once DECUP hardware issues are fixed
    xMessageBufferSend(out::tx_message_buffer.front_handle,
                       data(bytes),
                       size(bytes),
                       pdMS_TO_TICKS(task.timeout));

    uint8_t acks;
    auto const bytes_received{
      xMessageBufferReceive(out::rx_message_buffer.handle,
                            &acks,
                            sizeof(acks),
                            pdMS_TO_TICKS(task.timeout))};
    assert(bytes_received);
    */

    // Print incoming
    for (auto c : bytes) printf("%X ", c);

    /// \todo remove once DECUP hardware issues are fixed
    uint8_t acks;
    if (size(bytes) == 1uz) {
      // MX645 startbyte
      if (bytes[0uz] == 221u) acks = 2uz;
      else acks = 1uz;
    }
    // all other packets
    else
      acks = 2uz;

    printf(" -> %d\n", acks);
    return acks;
  }

  void done() final {}
};

/// \todo document
void transmit_response(uint8_t byte) {
  xStreamBufferSend(
    tx_stream_buffer.handle, &byte, sizeof(byte), pdMS_TO_TICKS(task.timeout));
}

/// Actual usb::decup_ein::task loop
void loop() {
  // auto const timeout{usb_receive_timeout2ms()};
  auto const timeout{task.timeout};
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(timeout)};

  ZsuLoad zsu_load{};

  for (;;) {
    // Return on timeout
    if (auto const now{xTaskGetTickCount()}; now >= then) return;
    // In case we got a byte, reset timeout
    else if (uint8_t byte; xStreamBufferReceive(rx_stream_buffer.handle,
                                                &byte,
                                                sizeof(byte),
                                                pdMS_TO_TICKS(task.timeout))) {
      then = now + pdMS_TO_TICKS(timeout);
      if (auto const resp{zsu_load.receive(byte)}) transmit_response(*resp);
    }
  }
}

}  // namespace

/// \todo document
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    //
    if (auto expected{State::Suspended};
        state.compare_exchange_strong(expected, State::DECUP_EIN)) {
      transmit_ok();
      /// \todo re-enable DECUP task once DECUP hardware issues are fixed
      // LOGI_TASK_RESUME(out::track::decup::task.handle);
      loop();
      /// \todo remove once DECUP hardware issues are fixed
      state.store(State::Suspended);
    }
    //
    else
      transmit_not_ok();

    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

}  // namespace usb::decup_ein