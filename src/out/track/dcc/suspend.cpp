/// Deinitialize peripherals when suspending DCC task
///
/// \file   out/track/dcc/suspend.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "suspend.hpp"
#include <driver/gpio.h>
#include <driver/uart.h>
#include "../suspend.hpp"

namespace out::track::dcc {

namespace {

///
esp_err_t deinit_bidi() {
  ESP_ERROR_CHECK(gpio_set_level(bidi_en_gpio_num, 0u));
  return uart_driver_delete(UART_NUM_1);
}

}  // namespace

///
esp_err_t suspend() {
  ESP_ERROR_CHECK(track::suspend());
  return deinit_bidi();
}

}  // namespace out::track::dcc