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

/// SUSIV2 protocol task function
///
/// \file   usb/ulf_susiv2/task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "task_function.hpp"
#include <ulf/susiv2.hpp>
#include "../tx_task_function.hpp"
#include "log.h"
#include "utility.hpp"

namespace usb::ulf_susiv2 {

namespace {

std::optional<std::span<uint8_t const>>
receive_susiv2_packet(std::span<uint8_t> stack) {
  auto const timeout{http_receive_timeout2ms()};
  size_t count{};

  for (;;) {
    // Receive single character
    auto const bytes_received{xStreamBufferReceive(
      rx_stream_buffer.handle, &stack[count], 1uz, pdMS_TO_TICKS(timeout))};
    count += bytes_received;
    if (!bytes_received || count > size(stack)) return std::nullopt;

    // Check if data contains a SUSIV2 frame, convert it inplace
    std::span<uint8_t const> frame{data(stack), count};
    auto const success{ulf::susiv2::frame2packet(frame)};
    if (!success) return std::nullopt;
    else if (*success) return frame;
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
      tx_stream_buffer.handle, data(stack), bytes_received, portMAX_DELAY);
}

/// \todo document
void loop() {
  std::array<uint8_t, ULF_SUSIV2_MAX_FRAME_SIZE> stack;
  while (auto const packet{receive_susiv2_packet(stack)}) {
    send_to_front(*packet);
    transmit_response(stack);
    if (return_on_exit(*packet)) return;
  }
}

} // namespace

/// \todo document
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    //
    if (auto expected{State::Suspended};
        state.compare_exchange_strong(expected, State::ULF_SUSIV2)) {
      transmit_ok();
      LOGI_TASK_RESUME(out::zusi::task.handle);
      loop();
    }
    //
    else
      transmit_not_ok();

    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

} // namespace usb::ulf_susiv2