/// Deinitialize peripherals when suspending ZUSI task
///
/// \file   out/zusi/suspend.cpp
/// \author Vincent Hamp
/// \date   27/03/2023

#include "suspend.hpp"
#include <driver/gpio.h>
#include <hal/gpio_ll.h>
#include <algorithm>
#include "../suspend.hpp"
#include "init.hpp"

namespace out::zusi {

namespace {

/// TODO
esp_err_t deinit_gpio() {
  ESP_ERROR_CHECK(gpio_set_direction(data_gpio_num, GPIO_MODE_OUTPUT));
  esp_rom_gpio_connect_out_signal(
    data_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  esp_rom_gpio_connect_out_signal(
    clock_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  ESP_ERROR_CHECK(gpio_set_level(data_gpio_num, 0u));
  ESP_ERROR_CHECK(gpio_set_level(clock_gpio_num, 0u));
  return gpio_set_level(enable_gpio_num, 0u);
}

}  // namespace

/// TODO
esp_err_t suspend() {
  ESP_ERROR_CHECK(deinit_gpio());
  return out::suspend();
}

}  // namespace out::zusi