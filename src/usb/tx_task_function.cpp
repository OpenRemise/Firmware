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

/// USB transmit task function
///
/// \file   usb/tx_task_function.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "tx_task_function.hpp"
#include <tinyusb.h>
#include <tusb_cdc_acm.h>
#include <array>
#include <span>
#include <string_view>
#include "log.h"

namespace usb {

/// Transmit "OK\r" to the CDC device queue
void transmit_ok() {
  static constexpr std::string_view ok{"OK\r"};
  xStreamBufferSend(tx_stream_buffer.handle, data(ok), size(ok), 0u);
}

/// Transmit "NOT OK\r" to the CDC device queue
void transmit_not_ok() {
  static constexpr std::string_view not_ok{"NOT OK\r"};
  xStreamBufferSend(tx_stream_buffer.handle, data(not_ok), size(not_ok), 0u);
}

namespace {

/// Receive data from usb::tx_stream_buffer and copy it to the stack
///
/// \param  stack Stack
/// \return Number of bytes received
size_t receive(std::span<uint8_t> stack) {
  return xStreamBufferReceive(
    tx_stream_buffer.handle, data(stack), size(stack), portMAX_DELAY);
}

/// Transmit data to the CDC device queue
///
/// \param  stack Stack
void transmit(std::span<uint8_t const> stack) {
  auto const bytes_to_transmit{size(stack)};
  for (auto i{0uz}; i < bytes_to_transmit;) {
    i += tinyusb_cdcacm_write_queue(
      TINYUSB_CDC_ACM_0, &stack[i], bytes_to_transmit - i);
  };
}

/// Flush CDC device
void flush() {
  if (tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0,
                                 pdMS_TO_TICKS(tx_task.timeout)) == ESP_FAIL)
    LOGE("USB CDC flush error");
}

} // namespace

/// USB transmit task function
///
/// Receive data from \ref tx_stream_buffer and transmit it to the CDC device
/// queue.
void tx_task_function(void*) {
  std::array<uint8_t, buffer_size> stack;
  for (;;) {
    auto const bytes_received{receive(stack)};
    transmit({cbegin(stack), bytes_received});
    flush();
  }
}

} // namespace usb