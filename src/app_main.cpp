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

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_random.h>
#include <esp_wifi.h>
#include <soc/uart_pins.h>
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

// wtf? 6.0.0 defines those publicly
extern "C" esp_err_t gpio_od_enable(gpio_num_t gpio_num);

/// ESP-IDF application entry point
extern "C" void app_main() {
  // Basic UART config on pins 38/39
  static constexpr uart_config_t uart_config{.baud_rate = 9600,
                                             .data_bits = UART_DATA_8_BITS,
                                             .parity = UART_PARITY_DISABLE,
                                             .stop_bits = UART_STOP_BITS_1,
                                             .flow_ctrl =
                                               UART_HW_FLOWCTRL_DISABLE,
                                             .source_clk = UART_SCLK_DEFAULT};
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 1024, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1,
                               GPIO_NUM_38, // TX
                               GPIO_NUM_39, // RX
                               UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));

  // Make GPIOs OD
  ESP_ERROR_CHECK(gpio_od_enable(GPIO_NUM_39));
  ESP_ERROR_CHECK(gpio_od_enable(GPIO_NUM_38));

  for (;;) {
    // Write random ASCII
    vTaskDelay(pdMS_TO_TICKS(1000u));
    char const cw{static_cast<char>(esp_random() % 95 + 32)};
    auto const bytes_written{uart_write_bytes(UART_NUM_1, &cw, sizeof(cw))};
    printf("wr %d-%c    ", bytes_written, cw);

    // Read it back
    vTaskDelay(pdMS_TO_TICKS(100u));
    char cr{};
    auto const bytes_read{uart_read_bytes(UART_NUM_1, &cr, sizeof(cr), 0u)};
    printf("rd %d-%c\n", bytes_read, cr);

    // Flush inputs
    uart_flush(UART_NUM_1);
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
