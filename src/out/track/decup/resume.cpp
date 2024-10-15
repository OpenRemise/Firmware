// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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