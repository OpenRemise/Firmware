// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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

}  // namespace

///
esp_err_t resume() {
  ESP_ERROR_CHECK(init_alarm());
  return init_gpio();
}

}  // namespace out::zusi
