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

/// Initialize peripherals when resuming DCC task
///
/// \file   drv/out/track/dcc/resume.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "resume.hpp"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <algorithm>

namespace drv::out::track::dcc {

/// \todo document
esp_err_t init_encoder(dcc_encoder_config_t const& encoder_cfg) {
  assert(!encoder);
  return rmt_new_dcc_encoder(&encoder_cfg, &encoder);
}

/// \todo document
esp_err_t init_rmt(rmt_tx_done_callback_t rmt_cb) {
  rmt_tx_event_callbacks_t cbs{.on_trans_done = rmt_cb};
  return rmt_tx_register_event_callbacks(channel, &cbs, NULL);
}

/// \todo document
esp_err_t init_alarm(gptimer_alarm_cb_t gptimer_cb) {
  gptimer_event_callbacks_t cbs{.on_alarm = gptimer_cb};
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
  ESP_ERROR_CHECK(gptimer_enable(gptimer));
  return gptimer_start(gptimer);
}

/// \todo document
esp_err_t init_bidi() {
  //
  static constexpr uart_config_t uart_cfg{
    .baud_rate = ::dcc::bidi::baudrate,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_driver_install(
    UART_NUM_1, SOC_UART_FIFO_LEN * 2, 0, 0, NULL, ESP_INTR_FLAG_IRAM));
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_cfg));
  return uart_set_pin(UART_NUM_1,
                      UART_PIN_NO_CHANGE,
                      bidi_rx_gpio_num,
                      UART_PIN_NO_CHANGE,
                      UART_PIN_NO_CHANGE);
}

/// \todo document
esp_err_t init_gpio() {
  ESP_ERROR_CHECK(gpio_set_level(enable_gpio_num, 1u));
  vTaskDelay(pdMS_TO_TICKS(20u));
  return gpio_set_level(n_force_low_gpio_num, 0u);
}

/// \todo document
esp_err_t resume(dcc_encoder_config_t const& encoder_cfg,
                 rmt_tx_done_callback_t rmt_cb,
                 gptimer_alarm_cb_t gptimer_cb) {
  ESP_ERROR_CHECK(init_encoder(encoder_cfg));
  ESP_ERROR_CHECK(init_rmt(rmt_cb));
  ESP_ERROR_CHECK(init_alarm(gptimer_cb));
  ESP_ERROR_CHECK(init_bidi());
  return init_gpio();
}

} // namespace drv::out::track::dcc
