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

///
///
/// \file   out/track/current_limit.cpp
/// \author Vincent Hamp
/// \date   15/02/2024

#include "current_limit.hpp"
#include <driver/gpio.h>

namespace out::track {

/// \todo document
CurrentLimit get_current_limit() {
  return static_cast<CurrentLimit>((gpio_get_level(ilim1_gpio_num) << 1u) |
                                   (gpio_get_level(ilim0_gpio_num) << 0u));
}

/// \todo document
esp_err_t set_current_limit(CurrentLimit current_limit) {
  switch (current_limit) {
    case CurrentLimit::_500mA:
      return gpio_set_level(ilim1_gpio_num, 0u) |
             gpio_set_level(ilim0_gpio_num, 0u);
    case CurrentLimit::_1300mA:
      return gpio_set_level(ilim1_gpio_num, 0u) |
             gpio_set_level(ilim0_gpio_num, 1u);
    case CurrentLimit::_2700mA:
      return gpio_set_level(ilim1_gpio_num, 1u) |
             gpio_set_level(ilim0_gpio_num, 0u);
    case CurrentLimit::_4100mA:
      return gpio_set_level(ilim1_gpio_num, 1u) |
             gpio_set_level(ilim0_gpio_num, 1u);
    default: return ESP_ERR_INVALID_ARG;
  }
}

} // namespace out::track
