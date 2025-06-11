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
/// \file   mw/ulf/decup_ein/task_function.cpp
/// \author Vincent Hamp
/// \date   04/05/2025

#include "task_function.hpp"
#include <ulf/decup_ein.hpp>
#include "intf/usb/tx_task_function.hpp"
#include "log.h"
#include "utility.hpp"

namespace mw::ulf::decup_ein {

using namespace ulf::decup_ein;

/// Receiver
class Decup : public ::ulf::decup_ein::rx::Base {
  uint8_t transmit(std::span<uint8_t const> bytes) final {
    uint8_t acks{};
    if (!xMessageBufferSend(drv::out::tx_message_buffer.front_handle,
                            data(bytes),
                            size(bytes),
                            pdMS_TO_TICKS(task.timeout)))
      return acks;
    else
      xMessageBufferReceive(drv::out::rx_message_buffer.handle,
                            &acks,
                            sizeof(acks),
                            pdMS_TO_TICKS(task.timeout));
    return acks;
  }
};

namespace {

/// Transmit response returned from receiver
///
/// \param byte Ack (`0x1C`) or nak (`0xFC`)
void transmit_response(uint8_t byte) {
  xStreamBufferSend(intf::usb::tx_stream_buffer.handle,
                    &byte,
                    sizeof(byte),
                    pdMS_TO_TICKS(intf::usb::tx_task.timeout));
}

/// Actual ulf::decup_ein::task loop
void loop() {
  Decup decup{};

  // Wait for RTS on
  auto then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};
  while (!intf::usb::rts && xTaskGetTickCount() < then)
    vTaskDelay(pdMS_TO_TICKS(100u));
  while (!xStreamBufferReset(intf::usb::rx_stream_buffer.handle)) {
    LOGW("Can't reset usb::rx_stream_buffer");
    vTaskDelay(pdMS_TO_TICKS(20u));
  }

  // While RTS
  while (intf::usb::rts && xTaskGetTickCount() < then) {
    // Receive single character
    uint8_t byte;
    if (!xStreamBufferReceive(intf::usb::rx_stream_buffer.handle,
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

/// ULF_DECUP_EIN task function
///
/// This task is created by the \ref usb::rx_task_function "USB receive task"
/// when a `DECUP_EIN\r` protocol string is received. Unfortunately, the
/// protocol itself is stateful and requires its own \ref ulf::decup_ein::Decup
/// "receiver class". An instance of this receiver on the stack takes care of
/// decoding the raw USB data to DECUP packets. Those packets are then
/// transmitted to drv::out::tx_message_buffer.
///
/// A special feature of this protocol is that the RTS line is used to switch
/// the track voltage on and off. For this reason, the RTS line is monitored and
/// its state is stored in a global variable \ref usb::rts.
///
/// At the end of an upload, the \ref usb::rx_task_function "USB receive task"
/// is resumed and this task destroys itself.
void task_function(void*) {
  // Switch to ULF_DECUP_EIN mode
  if (auto expected{State::Suspended};
      state.compare_exchange_strong(expected, State::ULF_DECUP_EIN)) {
    intf::usb::transmit_ok();
    LOGI_TASK_CREATE(drv::out::track::decup::task);
    loop();
  }
  // ... or not
  else
    intf::usb::transmit_not_ok();

  LOGI_TASK_RESUME(intf::usb::rx_task);
  LOGI_TASK_DESTROY();
}

} // namespace mw::ulf::decup_ein
