// Copyright (C) 2026 Vincent Hamp
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

///
///
/// \file   mw/display/service.cpp
/// \author Vincent Hamp
/// \date   08/01/2026

#include "service.hpp"
#include <driver/uart.h>
#include <esp_task.h>

namespace mw::display {

/// \todo document
Service::Service(std::function<std::string()> json_sys_get_request)
  : _json_sys_get_request{json_sys_get_request} {
  task.create(ztl::make_trampoline(this, &Service::taskFunction));
}

/// \todo document
[[noreturn]] void Service::taskFunction(void*) {
  std::string json{};
  for (;;) {
    json = _json_sys_get_request();
    uart_write_bytes(UART_NUM_0, data(json), size(json));
    vTaskDelay(pdMS_TO_TICKS(task.timeout));
  }
}

} // namespace mw::display
