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
#include "drv/anlg/init.hpp"
#include "drv/eth/init.hpp"
#include "drv/led/init.hpp"
#include "drv/out/init.hpp"
#include "drv/trace/init.hpp"
#include "drv/wifi/init.hpp"
#include "intf/dns/init.hpp"
#include "intf/http/init.hpp"
#include "intf/mdns/init.hpp"
#include "intf/udp/init.hpp"
#include "intf/usb/init.hpp"
#include "mem/nvs/init.hpp"
#include "mw/dcc/init.hpp"
#include "mw/disp/init.hpp"
#include "mw/ota/init.hpp"
#include "mw/roco/z21/init.hpp"
#include "mw/zimo/decup/init.hpp"
#include "mw/zimo/mdu/init.hpp"
#include "mw/zimo/ulf/init.hpp"
#include "mw/zimo/zusi/init.hpp"
#include "utility.hpp"

#include <esp_twai.h>
#include <esp_twai_onchip.h>

// Transmission completion callback
static IRAM_ATTR bool twai_tx_done_cb(twai_node_handle_t handle,
                                      twai_tx_done_event_data_t const* edata,
                                      void*) {
  if (!edata->is_tx_success) {
    DRAM_LOGW("Failed to transmit message, ID: 0x%X",
              edata->done_tx_frame->header.id);
  }
  return false; // No task wake required
}

// Reception complete callback
static bool twai_rx_cb(twai_node_handle_t handle,
                       twai_rx_done_event_data_t const* edata,
                       void*) {
  std::array<uint8_t, 8uz> buffer{};
  twai_frame_t rx_frame{.buffer = data(buffer), .buffer_len = size(buffer)};
  if (ESP_OK == twai_node_receive_from_isr(handle, &rx_frame))
    DRAM_LOGI("rx: %02X %02X %02X %02X %02X %02X %02X %02X\n",
              buffer[0uz],
              buffer[1uz],
              buffer[2uz],
              buffer[3uz],
              buffer[4uz],
              buffer[5uz],
              buffer[6uz],
              buffer[7uz]);
  return false;
}

// Node state callback
static bool IRAM_ATTR
twai_state_change_cb(twai_node_handle_t handle,
                     twai_state_change_event_data_t const* edata,
                     void*) {
  DRAM_LOGI("state changed: %d -> %d", edata->old_sta, edata->new_sta);
  return false;
}

// Error callback
static bool IRAM_ATTR twai_error_cb(twai_node_handle_t handle,
                                    twai_error_event_data_t const* edata,
                                    void*) {
  DRAM_LOGW("bus error: 0x%x", edata->err_flags.val);
  return false;
}

/// ESP-IDF application entry point
extern "C" void app_main() {
  static_assert(PRO_CPU_NUM == 0 && WIFI_TASK_CORE_ID == 0);
  static_assert(APP_CPU_NUM == 1);

  // Most important ones
  ESP_ERROR_CHECK(ipc_call_blocking(PRO_CPU_NUM, mem::nvs::init));
  static_assert(APP_CPU_NUM == mem::nvs::task.core_id);
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, drv::anlg::init));
  static_assert(APP_CPU_NUM == drv::anlg::adc_task.core_id &&
                APP_CPU_NUM == drv::anlg::temp_task.core_id);
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, drv::out::init));
  static_assert(APP_CPU_NUM == drv::out::susi::zimo::zusi::task.core_id &&
                APP_CPU_NUM == drv::out::track::dcc::task.core_id &&
                APP_CPU_NUM == drv::out::track::zimo::decup::task.core_id &&
                APP_CPU_NUM == drv::out::track::zimo::mdu::task.core_id);

  // Don't change initialization order
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, drv::led::init));
  if (auto const err{ipc_call_blocking(WIFI_TASK_CORE_ID, drv::eth::init)})
    ESP_ERROR_CHECK(ipc_call_blocking(WIFI_TASK_CORE_ID, drv::wifi::init));
  ESP_ERROR_CHECK(ipc_call_blocking(PRO_CPU_NUM, intf::http::init));
  ESP_ERROR_CHECK(ipc_call_blocking(PRO_CPU_NUM, intf::udp::init));
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, mw::dcc::init));
  static_assert(APP_CPU_NUM == mw::dcc::task.core_id);
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, mw::ota::init));
  static_assert(APP_CPU_NUM == mw::ota::task.core_id);
  ESP_ERROR_CHECK(ipc_call_blocking(PRO_CPU_NUM, mw::roco::z21::init));
  static_assert(APP_CPU_NUM == mw::roco::z21::task.core_id);
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, mw::zimo::zusi::init));
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, mw::zimo::decup::init));
  static_assert(APP_CPU_NUM == mw::zimo::decup::task.core_id);
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, mw::zimo::mdu::init));
  static_assert(APP_CPU_NUM == mw::zimo::mdu::task.core_id);
  static_assert(APP_CPU_NUM == mw::zimo::zusi::task.core_id);
  ESP_ERROR_CHECK(ipc_call_blocking(PRO_CPU_NUM, intf::dns::init));
  ESP_ERROR_CHECK(ipc_call_blocking(PRO_CPU_NUM, intf::mdns::init));

  // Either use U0RX and U0TX as trace outputs
#if defined(CONFIG_COMPILER_OPTIMIZATION_DEBUG)
  ESP_ERROR_CHECK(ipc_call_blocking(PRO_CPU_NUM, drv::trace::init));
  // ... or as UART display
#else
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, mw::disp::init));
  static_assert(APP_CPU_NUM == mw::disp::task.core_id);
#endif

  // Don't disable serial JTAG
#if !defined(CONFIG_USJ_ENABLE_USB_SERIAL_JTAG)
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, mw::zimo::ulf::init));
  static_assert(APP_CPU_NUM == mw::zimo::ulf::dcc_ein::task.core_id &&
                APP_CPU_NUM == mw::zimo::ulf::susiv2::task.core_id);
  ESP_ERROR_CHECK(ipc_call_blocking(APP_CPU_NUM, intf::usb::init));
  static_assert(APP_CPU_NUM == intf::usb::rx_task.core_id &&
                APP_CPU_NUM == intf::usb::tx_task.core_id);
#endif

  //
  // --
  //

  twai_node_handle_t node_hdl{};
  static constexpr twai_onchip_node_config_t node_config{
    .io_cfg = {.tx = GPIO_NUM_39,
               .rx = GPIO_NUM_38,
               .quanta_clk_out = GPIO_NUM_NC,
               .bus_off_indicator = GPIO_NUM_NC},
    .bit_timing = {.bitrate = 125000u},
    .tx_queue_depth = 5u};

  // Create a new TWAI controller driver instance
  ESP_ERROR_CHECK(twai_new_node_onchip(&node_config, &node_hdl));

  // Register transmission completion callback
  static constexpr twai_event_callbacks_t callbacks{
    .on_tx_done = twai_tx_done_cb,
    .on_rx_done = twai_rx_cb,
    .on_state_change = twai_state_change_cb,
    .on_error = twai_error_cb};
  ESP_ERROR_CHECK(
    twai_node_register_event_callbacks(node_hdl, &callbacks, NULL));

  // Start the TWAI controller
  ESP_ERROR_CHECK(twai_node_enable(node_hdl));

  for (;;) {
    uint32_t const id{
      1u << 28u |    // always 1
      0x0Au << 24u | // group (network)
      0x00u << 18u | // command (ping)
      0b00u << 16u | // mode (request)
      0u << 0u       // network id
    };

    std::array<uint8_t, 2uz> buffer{};

    twai_frame_t tx_frame = {
      .header =
        {
          .id = id,
          .ide = true,
        },
      .buffer = data(buffer),
      .buffer_len = size(buffer),
      .tx_queue_priority = 0u,
    };
    ESP_ERROR_CHECK(twai_node_transmit(node_hdl, &tx_frame, 500));
    ESP_ERROR_CHECK(twai_node_transmit_wait_all_done(node_hdl, -1));
    vTaskDelay(pdMS_TO_TICKS(2000u));
  }
}

// Assert that task names are unique and below max length
static_assert(std::invoke([] {
  std::array task_names{drv::anlg::adc_task.name,
                        drv::anlg::temp_task.name,
                        drv::out::susi::zimo::zusi::task.name,
                        drv::out::track::dcc::task.name,
                        drv::out::track::zimo::decup::task.name,
                        drv::out::track::zimo::mdu::task.name,
                        intf::usb::rx_task.name,
                        intf::usb::tx_task.name,
                        mem::nvs::task.name,
                        mw::dcc::task.name,
                        mw::ota::task.name,
                        mw::roco::z21::task.name,
                        mw::zimo::decup::task.name,
                        mw::zimo::mdu::task.name,
                        mw::zimo::ulf::dcc_ein::task.name,
                        mw::zimo::ulf::susiv2::task.name,
                        mw::zimo::zusi::task.name};
  std::ranges::sort(task_names, [](char const* a, char const* b) {
    return ztl::strcmp(a, b) < 0;
  });

  return
    // Names are unique
    std::unique(begin(task_names), end(task_names)) == cend(task_names) &&
    // ... and short enough
    std::ranges::all_of(task_names, [](char const* a) {
      return ztl::strlen(a) <= CONFIG_FREERTOS_MAX_TASK_NAME_LEN;
    });
}));
