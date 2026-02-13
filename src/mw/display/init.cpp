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
/// \file   mw/display/init.cpp
/// \author Vincent Hamp
/// \date   08/01/2026

#include "init.hpp"
#include <driver/uart.h>
#include <soc/uart_pins.h>
#include <memory>
#include "intf/http/sta/server.hpp"
#include "service.hpp"

namespace mw::display {

namespace {

std::unique_ptr<Service> service;

} // namespace

/// \todo document
esp_err_t init() {
  static constexpr uart_config_t uart_config{.baud_rate = CONFIG_MONITOR_BAUD,
                                             .data_bits = UART_DATA_8_BITS,
                                             .parity = UART_PARITY_DISABLE,
                                             .stop_bits = UART_STOP_BITS_1,
                                             .flow_ctrl =
                                               UART_HW_FLOWCTRL_DISABLE,
                                             .source_clk = UART_SCLK_DEFAULT};
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024, 1024, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0,
                               U0TXD_GPIO_NUM,
                               U0RXD_GPIO_NUM,
                               UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));

  if (intf::http::sta::server) {
    service = std::make_unique<Service>([]() -> std::string {
      return *intf::http::sta::server->sysGetRequest({});
    });
  }
  return ESP_OK;
}

} // namespace mw::display
