/// Initialize peripherals when resuming DCC task
///
/// \file   out/track/dcc/resume.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "resume.hpp"
#include <driver/uart.h>
#include <algorithm>
#include "../resume.hpp"

namespace out::track::dcc {

namespace {

///
esp_err_t init_encoder(dcc_encoder_config_t const& encoder_config) {
  assert(!encoder);
  return rmt_new_dcc_encoder(&encoder_config, &encoder);
}

///
esp_err_t init_bidi() {
  //
  static constexpr uart_config_t uart_config{
    .baud_rate = ::dcc::bidi::baudrate,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_driver_install(
    UART_NUM_1, SOC_UART_FIFO_LEN * 2, 0, 0, NULL, ESP_INTR_FLAG_IRAM));
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
  return uart_set_pin(UART_NUM_1,
                      UART_PIN_NO_CHANGE,
                      bidi_rx_gpio_num,
                      UART_PIN_NO_CHANGE,
                      UART_PIN_NO_CHANGE);
}

}  // namespace

///
esp_err_t resume(dcc_encoder_config_t const& encoder_config,
                 rmt_tx_done_callback_t rmt_cb,
                 gptimer_alarm_cb_t gptimer_cb) {
  ESP_ERROR_CHECK(init_encoder(encoder_config));
  ESP_ERROR_CHECK(init_bidi());
  return track::resume(rmt_cb, gptimer_cb);
}

}  // namespace out::track::dcc
