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

/// Initialize peripherals when resuming DECUP task
///
/// \file   out/track/decup/resume.cpp
/// \author Vincent Hamp
/// \date   14/08/2024

#include "resume.hpp"
#include <driver/gptimer.h>

namespace out::track::decup {

/// \todo document
esp_err_t init_encoder(decup_encoder_config_t const& encoder_config) {
  assert(!encoder);
  return rmt_new_decup_encoder(&encoder_config, &encoder);
}

namespace {

/// \todo document
esp_err_t init_alarm() {
  ESP_ERROR_CHECK(gptimer_enable(gptimer));
  return gptimer_start(gptimer);
}

/// \todo document
esp_err_t init_gpio(gpio_isr_t gpio_isr_handler) {
  ESP_ERROR_CHECK(gpio_isr_handler_add(ack_gpio_num, gpio_isr_handler, NULL));
  return gpio_set_level(enable_gpio_num, 1u);
}

}  // namespace

/// \todo document
esp_err_t resume(decup_encoder_config_t const& encoder_config,
                 gpio_isr_t gpio_isr_handler) {
  ESP_ERROR_CHECK(init_encoder(encoder_config));
  ESP_ERROR_CHECK(init_alarm());
  return init_gpio(gpio_isr_handler);
}

}  // namespace out::track::decup