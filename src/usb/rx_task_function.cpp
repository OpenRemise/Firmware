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

/// USB receive task function
///
/// \file   usb/rx_task_function.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "rx_task_function.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <string_view>
#include <ulf/com.hpp>
#include "log.h"

#if CONFIG_IDF_TARGET_ESP32S3
#  include <esp_app_desc.h>
#endif

namespace usb {

using namespace std::literals;

/// Execute ping command
///
/// Transmit device name and semantic version to the CDC device queue.
void transmit_ping() {
#if CONFIG_IDF_TARGET_ESP32S3
  auto const app_desc{esp_app_get_description()};
  auto const ping{ulf::com::ping("OpenRemise", app_desc->version + 1, 'D')};
  xStreamBufferSend(tx_stream_buffer.handle, data(ping), size(ping), 0u);
#endif
}

/// Check if any protocol task is active
///
/// \return true if any protocol task is active
/// \return false if no protocol task is active
bool any_protocol_task_active() {
  return eTaskGetState(ulf_dcc_ein::task.handle) < eSuspended ||
         eTaskGetState(ulf_decup_ein::task.handle) < eSuspended ||
         eTaskGetState(ulf_susiv2::task.handle) < eSuspended;
}

namespace {

/// Actual usb::rx_task loop
void loop() {
  std::array<char, buffer_size> stack;
  size_t count{};

  for (;;) {
    // Read single character
    while (!xStreamBufferReceive(rx_stream_buffer.handle,
                                 &stack[count],
                                 1uz,
                                 pdMS_TO_TICKS(rx_task.timeout)))
      count = 0uz;
    count += 1uz;
    std::string_view const str{data(stack), count};

    // Maybe command
    if (auto const cmd{ulf::com::str2cmd(str)}) {
      // Not enough characters yet
      if (cmd == std::nullopt) continue;
      // Ping
      else if (cmd == "PING\r"sv) transmit_ping();
      // Resume ULF_DCC_EIN protocol tasks
      else if (cmd == "DCC_EIN\r"sv) {
        LOGI_TASK_RESUME(ulf_dcc_ein::task.handle);
        break;
      }
      // Resume ULF_DECUP_EIN protocol tasks
      else if (cmd == "DECUP_EIN\r"sv) {
        LOGI_TASK_RESUME(ulf_decup_ein::task.handle);
        break;
      }
      // Resume ULF_MDU_EIN protocol tasks
      else if (cmd == "MDU_EIN\r"sv)
        LOGW("MDU_EIN protocol not implemented");
      // Resume ULF_SUSIV2 protocol tasks
      else if (cmd == "SUSIV2\r"sv) {
        LOGI_TASK_RESUME(ulf_susiv2::task.handle);
        break;
      }
    }

    count = 0uz;
  }
}

/// Wait until all \ref protocol_tasks are suspended
void wait_for_all_protocol_tasks_to_suspend() {
  while (any_protocol_task_active()) vTaskDelay(pdMS_TO_TICKS(rx_task.timeout));
}

} // namespace

/// USB receive task function
///
/// Scan the CDC character stream for protocol entry strings. Once a supported
/// string is detected the protocol tasks are resumed and the task suspends
/// itself.
void rx_task_function(void*) {
  for (;;) {
    loop();
    LOGI_TASK_SUSPEND(rx_task.handle);
    wait_for_all_protocol_tasks_to_suspend();
  }
}

} // namespace usb