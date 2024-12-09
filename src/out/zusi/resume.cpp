// Copyright (C) 2024 Vincent Hamp
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

/// Initialize peripherals when resuming ZUSI task
///
/// \file   out/zusi/resume.cpp
/// \author Vincent Hamp
/// \date   27/03/2023

#include "resume.hpp"
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <algorithm>
#include "init.hpp"

namespace out::zusi {

namespace {

/// \todo document
esp_err_t init_alarm() {
  ESP_ERROR_CHECK(gptimer_enable(gptimer));
  return gptimer_start(gptimer);
}

///
esp_err_t init_gpio() { return gpio_set_level(enable_gpio_num, 1u); }

} // namespace

///
esp_err_t resume() {
  ESP_ERROR_CHECK(init_alarm());
  return init_gpio();
}

} // namespace out::zusi
