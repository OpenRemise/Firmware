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

/// ESP-IDF application entry point
///
/// \file   app_main.cpp
/// \author Vincent Hamp
/// \date   26/12/2022

#include <esp_wifi.h>
#include "analog/init.hpp"
#include "dcc/init.hpp"
#include "decup/init.hpp"
#include "http/init.hpp"
#include "mdu/init.hpp"
#include "mem/init.hpp"
#include "ota/init.hpp"
#include "out/init.hpp"
#include "trace.hpp"
#include "udp/init.hpp"
#include "usb/init.hpp"
#include "utility.hpp"
#include "wifi/init.hpp"
#include "z21/init.hpp"
#include "zusi/init.hpp"

/// ESP-IDF application entry point
extern "C" void app_main() {
  static_assert(WIFI_TASK_CORE_ID == 0);

  // Most important ones
  ESP_ERROR_CHECK(invoke_on_core(0, trace::init));
  ESP_ERROR_CHECK(invoke_on_core(0, mem::init));
  ESP_ERROR_CHECK(invoke_on_core(1, analog::init, 1));
  ESP_ERROR_CHECK(invoke_on_core(1, out::init, 1));

  // Don't change initialization order
  ESP_ERROR_CHECK(invoke_on_core(0, wifi::init));
  ESP_ERROR_CHECK(invoke_on_core(0, http::init));
  ESP_ERROR_CHECK(invoke_on_core(0, udp::init));
  ESP_ERROR_CHECK(invoke_on_core(1, dcc::init, 1));
  ESP_ERROR_CHECK(invoke_on_core(1, decup::init, 1));
  ESP_ERROR_CHECK(invoke_on_core(1, mdu::init, 1));
  ESP_ERROR_CHECK(invoke_on_core(1, ota::init, 1));
  ESP_ERROR_CHECK(invoke_on_core(0, z21::init, 0));
  ESP_ERROR_CHECK(invoke_on_core(1, zusi::init, 1));

  // Don't disable serial JTAG
#if !defined(CONFIG_USJ_ENABLE_USB_SERIAL_JTAG)
  ESP_ERROR_CHECK(invoke_on_core(1, usb::init, 1));
#endif

  // char* buffer{new char[2048uz]};
  for (;;) {
    // vTaskGetRunTimeStats(buffer);
    // printf("%s\n", buffer);
    // esp_intr_dump(stdout);
    vTaskDelay(pdMS_TO_TICKS(5000u));
  }
}
