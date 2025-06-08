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
#include "led/init.hpp"
#include "mdns/init.hpp"
#include "mdu/init.hpp"
#include "mem/init.hpp"
#include "ota/init.hpp"
#include "out/init.hpp"
#include "trace/init.hpp"
#include "udp/init.hpp"
#include "ulf/init.hpp"
#include "usb/init.hpp"
#include "utility.hpp"
#include "wifi/init.hpp"
#include "z21/init.hpp"
#include "zusi/init.hpp"

/// ESP-IDF application entry point
extern "C" void app_main() {
  static_assert(PRO_CPU_NUM == 0 && WIFI_TASK_CORE_ID == 0);
  static_assert(APP_CPU_NUM == 1);

  // Most important ones
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, trace::init));
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, mem::init));
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, analog::init));
  static_assert(APP_CPU_NUM == analog::adc_task.core_id &&
                APP_CPU_NUM == analog::temp_task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, out::init));
  static_assert(APP_CPU_NUM == out::track::dcc::task.core_id &&
                APP_CPU_NUM == out::track::decup::task.core_id &&
                APP_CPU_NUM == out::track::mdu::task.core_id &&
                APP_CPU_NUM == out::zusi::task.core_id);

  // Don't change initialization order
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, led::init));
  ESP_ERROR_CHECK(invoke_on_core(WIFI_TASK_CORE_ID, wifi::init));
  static_assert(WIFI_TASK_CORE_ID == wifi::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, http::init));
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, udp::init));
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, dcc::init));
  static_assert(APP_CPU_NUM == dcc::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, decup::init));
  static_assert(APP_CPU_NUM == decup::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, mdu::init));
  static_assert(APP_CPU_NUM == mdu::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, ota::init));
  static_assert(APP_CPU_NUM == ota::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, z21::init));
  static_assert(APP_CPU_NUM == z21::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, zusi::init));
  static_assert(APP_CPU_NUM == zusi::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, mdns::init));

  // Don't disable serial JTAG
#if !defined(CONFIG_USJ_ENABLE_USB_SERIAL_JTAG)
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, ulf::init, 1));
  static_assert(APP_CPU_NUM == ulf::dcc_ein::task.core_id &&
                APP_CPU_NUM == ulf::decup_ein::task.core_id &&
                APP_CPU_NUM == ulf::susiv2::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, usb::init));
  static_assert(APP_CPU_NUM == usb::rx_task.core_id &&
                APP_CPU_NUM == usb::tx_task.core_id);
#endif
}
