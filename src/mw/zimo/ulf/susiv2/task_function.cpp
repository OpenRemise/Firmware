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
/// \file   mw/zimo/ulf/susiv2/task_function.cpp
/// \author Vincent Hamp
/// \date   04/05/2025

#include "task_function.hpp"
#include <ulf/susiv2.hpp>
#include "intf/usb/tx_task_function.hpp"
#include "log.h"
#include "utility.hpp"

namespace mw::zimo::ulf::susiv2 {

using namespace ::ulf::susiv2;

namespace {

/// Receive ZUSI packet from SUSIV2 frame
///
/// \param  stack   Stack
/// \param  timeout Timeout
/// \retval std::span<uint8_t const> ZUSI packet from SUSIV2 frame
/// \retval std::nullopt on timeout
std::optional<std::span<uint8_t const>>
receive_zusi_packet(std::span<uint8_t> stack, uint32_t timeout) {
  size_t count{};

  for (;;) {
    // Receive single character
    auto const bytes_received{
      xStreamBufferReceive(intf::usb::rx_stream_buffer.handle,
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

/// Send ZUSI packet to drv::out::tx_message_buffer front
///
/// \param  stack Stack
void send_to_front(std::span<uint8_t const> stack) {
  xMessageBufferSend(drv::out::tx_message_buffer.front_handle,
                     data(stack),
                     size(stack),
                     portMAX_DELAY);
}

/// Check if the stack contains an exit command
///
/// \param  stack Stack
/// \retval true  Stack contains an exit command
/// \retval false Stack does not contain an exit command
bool return_on_exit(std::span<uint8_t const> stack) {
  return size(stack) &&
         static_cast<::zusi::Command>(stack.front()) == ::zusi::Command::Exit;
}

/// Transmit response
///
/// \param  stack Stack
void transmit_response(std::span<uint8_t> stack) {
  if (auto const bytes_received{
        xMessageBufferReceive(drv::out::rx_message_buffer.handle,
                              data(stack),
                              size(stack),
                              portMAX_DELAY)})
    xStreamBufferSend(intf::usb::tx_stream_buffer.handle,
                      data(stack),
                      bytes_received,
                      portMAX_DELAY);
}

/// task_function() loop
void loop() {
  auto const timeout{http_receive_timeout2ms()};
  std::array<uint8_t, ULF_SUSIV2_MAX_FRAME_SIZE> stack;
  while (auto const packet{receive_zusi_packet(stack, timeout)}) {
    send_to_front(*packet);
    transmit_response(stack);
    if (return_on_exit(*packet)) return;
  }
}

} // namespace

/// ULF_SUSIV2 task function
///
/// This task is created by the \ref usb::rx_task_function "USB receive task"
/// when a `SUSIV2\r` protocol string is received. Once running the task scans
/// the CDC character stream for SUSIV2 frames, converts them to ZUSI packets
/// and transmits them to drv::out::tx_message_buffer.
///
/// Upon receiving an exit command the \ref usb::rx_task_function
/// "USB receive task" is created and this task destroys itself.
[[noreturn]] void task_function(void*) {
  // Switch to ULF_SUSIV2 mode
  if (auto expected{State::Suspended};
      state.compare_exchange_strong(expected, State::ULF_SUSIV2)) {
    intf::usb::transmit_ok();
    LOGI_TASK_CREATE(drv::out::susi::zimo::zusi::task);
    loop();
  }
  // ... or not
  else
    intf::usb::transmit_not_ok();

  LOGI_TASK_CREATE(intf::usb::rx_task);
  LOGI_TASK_DESTROY();
}

} // namespace mw::zimo::ulf::susiv2
