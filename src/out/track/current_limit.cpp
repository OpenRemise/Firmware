///
///
/// \file   out/track/current_limit.cpp
/// \author Vincent Hamp
/// \date   15/02/2024

#pragma once

#include "current_limit.hpp"
#include <driver/gpio.h>

namespace out::track {

/// TODO
CurrentLimit get_current_limit() {
  return static_cast<CurrentLimit>((gpio_get_level(isel1_gpio_num) << 1u) |
                                   (gpio_get_level(isel0_gpio_num) << 0u));
}

/// TODO
esp_err_t set_current_limit(CurrentLimit current_limit) {
  switch (current_limit) {
    case CurrentLimit::_500mA:
      return gpio_set_level(isel1_gpio_num, 0u) |
             gpio_set_level(isel0_gpio_num, 0u);
    case CurrentLimit::_1600mA:
      return gpio_set_level(isel1_gpio_num, 0u) |
             gpio_set_level(isel0_gpio_num, 1u);
    case CurrentLimit::_3000mA:
      return gpio_set_level(isel1_gpio_num, 1u) |
             gpio_set_level(isel0_gpio_num, 0u);
    case CurrentLimit::_4100mA:
      return gpio_set_level(isel1_gpio_num, 1u) |
             gpio_set_level(isel0_gpio_num, 1u);
    default: return ESP_ERR_INVALID_ARG;
  }
}

}  // namespace out::track