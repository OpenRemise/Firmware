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
/// \file   intf/usb/rx_task_function.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "rx_task_function.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <string_view>
#include <ulf/com.hpp>
#include "log.h"
#include "tx_task_function.hpp"

#if CONFIG_IDF_TARGET_ESP32S3
#  include <esp_app_desc.h>
#endif

namespace intf::usb {

using namespace std::literals;

/// Check if any USB service task is active
///
/// \retval true if any service task is active
/// \retval false if no service task is active
bool any_service_task_active() {
  return xTaskGetHandle(mw::zimo::ulf::dcc_ein::task.name) ||
         xTaskGetHandle(mw::zimo::ulf::susiv2::task.name);
}

/// Execute ping command
///
/// Transmit device name and semantic version to the CDC device queue.
void transmit_ping() {
#if CONFIG_IDF_TARGET_ESP32S3
  auto const app_desc{esp_app_get_description()};
  auto const ping{ulf::com::ping("OpenRemise", app_desc->version + 1)};
  xStreamBufferSend(tx_stream_buffer.handle, data(ping), size(ping), 0u);
#endif
}

namespace {

/// rx_task_function() loop
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
      // Create ULF_DCC_EIN task
      else if (cmd == "DCC_EIN\r"sv) {
        LOGI_TASK_CREATE(mw::zimo::ulf::dcc_ein::task);
        break;
      }
      // ULF_DECUP_EIN not supported
      else if (cmd == "DECUP_EIN\r"sv) {
        LOGW("DECUP_EIN not supported");
        transmit_not_ok();
      }
      // Create ULF_MDU_EIN task
      else if (cmd == "MDU_EIN\r"sv) {
        LOGW("MDU_EIN not implemented");
        transmit_not_ok();
      }
      // Create ULF_SUSIV2 task
      else if (cmd == "SUSIV2\r"sv) {
        LOGI_TASK_CREATE(mw::zimo::ulf::susiv2::task);
        break;
      }
    }

    count = 0uz;
  }
}

/// Wait until all service tasks are suspended
void wait_for_all_service_tasks_to_suspend() {
  while (any_service_task_active()) vTaskDelay(pdMS_TO_TICKS(rx_task.timeout));
}

} // namespace

/// USB receive task function
///
/// The receive task scans the CDC character stream for commands. Those commands
/// could either be general ones (e.g. `PING\r`) or protocol entry strings. When
/// a protocol entry string is detected the corrsponding task is created and the
/// receive task destroys itself.
///
/// Currently supported protocols are:
/// - [ULF_DCC_EIN](https://github.com/ZIMO-Elektronik/ULF_DCC_EIN)
/// - [ULF_SUSIV2](https://github.com/ZIMO-Elektronik/ULF_SUSIV2)
[[noreturn]] void rx_task_function(void*) {
  wait_for_all_service_tasks_to_suspend();
  loop();
  LOGI_TASK_DESTROY();
}

} // namespace intf::usb
