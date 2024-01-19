/// SUSIV2 protocol transmit task function
///
/// \file   usb/susiv2/tx_task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "tx_task_function.hpp"
#include <zusi/zusi.hpp>
#include "log.h"

namespace usb::susiv2 {

namespace {

/// TODO
std::optional<std::span<uint8_t>> receive(std::span<uint8_t> stack) {
  if (auto const bytes_received{
        xMessageBufferReceive(out::rx_message_buffer.handle,
                              data(stack),
                              size(stack),
                              pdMS_TO_TICKS(tx_task.timeout))})
    return stack.subspan(0uz, bytes_received);
  else return std::nullopt;
}

/// TODO
void transmit(std::span<uint8_t const> stack) {
  xStreamBufferSend(tx_stream_buffer.handle,
                    data(stack),
                    size(stack),
                    pdMS_TO_TICKS(tx_task.timeout));
}

/// TODO
void loop() {
  ::zusi::Buffer<buffer_size> stack;
  while (eTaskGetState(rx_task.handle) < eSuspended) {
    if (auto const msg{receive(stack)}) transmit(*msg);
  }
}

}  // namespace

/// TODO
void tx_task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(tx_task.handle);
    loop();
    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

}  // namespace usb::susiv2