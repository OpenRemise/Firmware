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
#include "drv/analog/init.hpp"
#include "drv/led/init.hpp"
#include "drv/out/init.hpp"
#include "drv/trace/init.hpp"
#include "drv/wifi/init.hpp"
#include "intf/http/init.hpp"
#include "intf/mdns/init.hpp"
#include "intf/udp/init.hpp"
#include "intf/usb/init.hpp"
#include "mem/init.hpp"
#include "mw/dcc/init.hpp"
#include "mw/decup/init.hpp"
#include "mw/mdu/init.hpp"
#include "mw/ota/init.hpp"
#include "mw/ulf/init.hpp"
#include "mw/z21/init.hpp"
#include "mw/zusi/init.hpp"
#include "utility.hpp"

/// ESP-IDF application entry point
extern "C" void app_main() {
  static_assert(PRO_CPU_NUM == 0 && WIFI_TASK_CORE_ID == 0);
  static_assert(APP_CPU_NUM == 1);

  // Most important ones
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, drv::trace::init));
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, mem::init));
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, drv::analog::init));
  static_assert(APP_CPU_NUM == drv::analog::adc_task.core_id &&
                APP_CPU_NUM == drv::analog::temp_task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, drv::out::init));
  static_assert(APP_CPU_NUM == drv::out::track::dcc::task.core_id &&
                APP_CPU_NUM == drv::out::track::decup::task.core_id &&
                APP_CPU_NUM == drv::out::track::mdu::task.core_id &&
                APP_CPU_NUM == drv::out::zusi::task.core_id);

  // Don't change initialization order
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, drv::led::init));
  ESP_ERROR_CHECK(invoke_on_core(WIFI_TASK_CORE_ID, drv::wifi::init));
  static_assert(WIFI_TASK_CORE_ID == drv::wifi::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, intf::http::init));
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, intf::udp::init));
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, mw::dcc::init));
  static_assert(APP_CPU_NUM == mw::dcc::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, mw::decup::init));
  static_assert(APP_CPU_NUM == mw::decup::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, mw::mdu::init));
  static_assert(APP_CPU_NUM == mw::mdu::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, mw::ota::init));
  static_assert(APP_CPU_NUM == mw::ota::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, mw::z21::init));
  static_assert(APP_CPU_NUM == mw::z21::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, mw::zusi::init));
  static_assert(APP_CPU_NUM == mw::zusi::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(PRO_CPU_NUM, intf::mdns::init));

  // Don't disable serial JTAG
#if !defined(CONFIG_USJ_ENABLE_USB_SERIAL_JTAG)
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, mw::ulf::init));
  static_assert(APP_CPU_NUM == mw::ulf::dcc_ein::task.core_id &&
                APP_CPU_NUM == mw::ulf::decup_ein::task.core_id &&
                APP_CPU_NUM == mw::ulf::susiv2::task.core_id);
  ESP_ERROR_CHECK(invoke_on_core(APP_CPU_NUM, intf::usb::init));
  static_assert(APP_CPU_NUM == intf::usb::rx_task.core_id &&
                APP_CPU_NUM == intf::usb::tx_task.core_id);
#endif
}
