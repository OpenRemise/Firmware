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

/// Trace (IO pins to toggle for debug purposes)
///
/// \file   trace/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <driver/gpio.h>

namespace trace {

/// \todo document
esp_err_t init() {
  static constexpr gpio_config_t io_conf{.pin_bit_mask = 1ull << GPIO_NUM_2 |
                                                         1ull << GPIO_NUM_1,
                                         .mode = GPIO_MODE_OUTPUT,
                                         .pull_up_en = GPIO_PULLUP_DISABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE};
  return gpio_config(&io_conf);
}

} // namespace trace
