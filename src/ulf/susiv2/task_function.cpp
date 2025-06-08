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

/// ULF_SUSIV2 task function
///
/// \file   ulf/susiv2/task_function.cpp
/// \author Vincent Hamp
/// \date   04/05/2025

#include "task_function.hpp"
#include <ulf/susiv2.hpp>
#include "log.h"
#include "usb/tx_task_function.hpp"
#include "utility.hpp"

namespace ulf::susiv2 {

namespace {

std::optional<std::span<uint8_t const>>
receive_susiv2_frame(std::span<uint8_t> stack, uint32_t timeout) {
  size_t count{};

  for (;;) {
    // Receive single character
    auto const bytes_received{xStreamBufferReceive(usb::rx_stream_buffer.handle,
                                                   &stack[count],
                                                   1uz,
                                                   pdMS_TO_TICKS(timeout))};
    count += bytes_received;
    if (!bytes_received || count > size(stack)) return std::nullopt;

    // Check if data contains a SUSIV2 frame, convert it inplace
    if (auto const packet{ulf::susiv2::frame2packet({data(stack), count})};
        !packet)
      return std::nullopt;
    else if (*packet) return **packet;
  }
}

/// \todo document
void send_to_front(std::span<uint8_t const> stack) {
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(stack),
                     size(stack),
                     portMAX_DELAY);
}

/// \todo document
bool return_on_exit(std::span<uint8_t const> stack) {
  return size(stack) &&
         static_cast<zusi::Command>(stack.front()) == zusi::Command::Exit;
}

/// \todo document
void transmit_response(std::span<uint8_t> stack) {
  if (auto const bytes_received{
        xMessageBufferReceive(out::rx_message_buffer.handle,
                              data(stack),
                              size(stack),
                              portMAX_DELAY)})
    xStreamBufferSend(
      usb::tx_stream_buffer.handle, data(stack), bytes_received, portMAX_DELAY);
}

/// \todo document
void loop() {
  auto const timeout{http_receive_timeout2ms()};
  std::array<uint8_t, ULF_SUSIV2_MAX_FRAME_SIZE> stack;
  while (auto const frame{receive_susiv2_frame(stack, timeout)}) {
    send_to_front(*frame);
    transmit_response(stack);
    if (return_on_exit(*frame)) return;
  }
}

} // namespace

/// \todo document
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND();

    //
    if (auto expected{State::Suspended};
        state.compare_exchange_strong(expected, State::ULF_SUSIV2)) {
      usb::transmit_ok();
      LOGI_TASK_RESUME(out::zusi::task.handle);
      loop();
    }
    //
    else
      usb::transmit_not_ok();

    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

} // namespace ulf::susiv2
