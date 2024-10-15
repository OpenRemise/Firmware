// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// SUSIV2 protocol task function
///
/// \file   usb/susiv2/task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "task_function.hpp"
#include <zusi/zusi.hpp>
#include "../tx_task_function.hpp"
#include "log.h"

namespace usb::susiv2 {

/// \todo document this shit needs to be in a SUSIV2 lib
std::optional<std::span<uint8_t>>
receive_susiv2_command(std::span<uint8_t> stack) {
  size_t count{};

  // Command is always 6 bytes
  count += xStreamBufferReceive(
    rx_stream_buffer.handle, &stack[count], 6uz, pdMS_TO_TICKS(task.timeout));
  if (count != 6uz) return std::nullopt;

  // Data size depends on command
  /// \todo this is horrible, DRY?
  switch (static_cast<zusi::Command>(stack[5uz])) {
    case zusi::Command::None: break;

    case zusi::Command::CvRead:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        6uz,
                                        pdMS_TO_TICKS(task.timeout));
          count != 6uz + 6uz)
        return std::nullopt;
      break;

    case zusi::Command::CvWrite:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        7uz,
                                        pdMS_TO_TICKS(task.timeout));
          count != 6uz + 7uz)
        return std::nullopt;
      break;

    case zusi::Command::ZppErase:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        3uz,
                                        pdMS_TO_TICKS(task.timeout));
          count != 6uz + 3uz)
        return std::nullopt;
      break;

    case zusi::Command::ZppWrite:
      // USB packets only come 64B at a time...
      while (count < size(stack)) {
        auto const bytes_received{
          xStreamBufferReceive(rx_stream_buffer.handle,
                               &stack[count],
                               size(stack) - count,
                               pdMS_TO_TICKS(task.timeout))};
        if (!bytes_received) return std::nullopt;
        count += bytes_received;
      }
      break;

    case zusi::Command::Features:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        1uz,
                                        pdMS_TO_TICKS(task.timeout));
          count != 6uz + 1uz)
        return std::nullopt;
      break;

    case zusi::Command::Exit:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        4uz,
                                        pdMS_TO_TICKS(task.timeout));
          count != 6uz + 4uz)
        return std::nullopt;
      break;

    /// \todo implement encrypt
    case zusi::Command::Encrypt:
      LOGW("'Encrypt' command not implemented");
      break;
  }

  //
  return stack.subspan(5uz, count);
}

namespace {

/// \todo document
void send_to_front(std::span<uint8_t> stack) {
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(stack),
                     size(stack),
                     portMAX_DELAY);
}

/// \todo document
bool return_on_exit(std::span<uint8_t> stack) {
  return size(stack) &&
         static_cast<zusi::Command>(stack.front()) == zusi::Command::Exit;
}

/// \todo document
void transmit_response(std::span<uint8_t> stack) {
  if (auto const bytes_received{
        xMessageBufferReceive(out::rx_message_buffer.handle,
                              data(stack),
                              size(stack),
                              pdMS_TO_TICKS(task.timeout))})
    xStreamBufferSend(tx_stream_buffer.handle,
                      data(stack),
                      bytes_received,
                      pdMS_TO_TICKS(task.timeout));
}

/// \todo document
void loop() {
  ::zusi::Buffer<buffer_size> stack;
  while (auto const cmd{receive_susiv2_command(stack)}) {
    send_to_front(*cmd);
    transmit_response(stack);
    if (return_on_exit(*cmd)) return;
  }
}

}  // namespace

/// \todo document
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    //
    if (auto expected{State::Suspended};
        state.compare_exchange_strong(expected, State::SUSIV2)) {
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

}  // namespace usb::susiv2