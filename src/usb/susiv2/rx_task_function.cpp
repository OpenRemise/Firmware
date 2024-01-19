/// SUSIV2 protocol receive task function
///
/// \file   usb/susiv2/rx_task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "rx_task_function.hpp"
#include <zusi/zusi.hpp>
#include "../tx_task_function.hpp"
#include "log.h"

namespace usb::susiv2 {

/// TODO
std::optional<std::span<uint8_t>>
receive_susiv2_command(std::span<uint8_t> stack) {
  size_t count{};

  // Command is always 6 bytes
  count += xStreamBufferReceive(rx_stream_buffer.handle,
                                &stack[count],
                                6uz,
                                pdMS_TO_TICKS(rx_task.timeout));
  if (count != 6uz) return std::nullopt;

  // Data size depends on command (TODO DRY...)
  switch (static_cast<zusi::Command>(stack[5uz])) {
    case zusi::Command::None: break;

    case zusi::Command::CvRead:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        6uz,
                                        pdMS_TO_TICKS(rx_task.timeout));
          count != 6uz + 6uz)
        return std::nullopt;
      break;

    case zusi::Command::CvWrite:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        7uz,
                                        pdMS_TO_TICKS(rx_task.timeout));
          count != 6uz + 7uz)
        return std::nullopt;
      break;

    case zusi::Command::ZppErase:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        3uz,
                                        pdMS_TO_TICKS(rx_task.timeout));
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
                               pdMS_TO_TICKS(rx_task.timeout))};
        if (!bytes_received) return std::nullopt;
        count += bytes_received;
      }
      break;

    case zusi::Command::Features:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        1uz,
                                        pdMS_TO_TICKS(rx_task.timeout));
          count != 6uz + 1uz)
        return std::nullopt;
      break;

    case zusi::Command::Exit:
      if (count += xStreamBufferReceive(rx_stream_buffer.handle,
                                        &stack[count],
                                        4uz,
                                        pdMS_TO_TICKS(rx_task.timeout));
          count != 6uz + 4uz)
        return std::nullopt;
      break;

    case zusi::Command::Encrypt:
      LOGW("'Encrypt' command not implemented");  // TODO
      break;
  }

  //
  return stack.subspan(5uz, count);
}

namespace {

/// TODO
void transmit(std::span<uint8_t> stack) {
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(stack),
                     size(stack),
                     portMAX_DELAY);
}

/// TODO
bool return_on_exit(std::span<uint8_t> stack) {
  return size(stack) &&
         static_cast<zusi::Command>(stack.front()) == zusi::Command::Exit;
}

/// TODO
void loop() {
  ::zusi::Buffer<buffer_size> stack;
  while (auto const cmd{receive_susiv2_command(stack)}) {
    transmit(*cmd);
    if (return_on_exit(*cmd)) return;
  }
}

}  // namespace

/// TODO
void rx_task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(rx_task.handle);
    if (auto expected{Mode::Suspended};
        mode.compare_exchange_strong(expected, Mode::SUSIV2)) {
      transmit_ok();
      LOGI_TASK_RESUME(out::zusi::task.handle);
      loop();
    } else transmit_not_ok();
    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

}  // namespace usb::susiv2