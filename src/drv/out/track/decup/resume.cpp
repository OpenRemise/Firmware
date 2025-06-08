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

/// Initialize peripherals when resuming DECUP task
///
/// \file   drv/out/track/decup/resume.cpp
/// \author Vincent Hamp
/// \date   14/08/2024

#include "resume.hpp"
#include <driver/gptimer.h>

namespace drv::out::track::decup {

/// \todo document
esp_err_t init_encoder(decup_encoder_config_t const& encoder_config) {
  assert(!encoder);
  return rmt_new_decup_encoder(&encoder_config, &encoder);
}

namespace {

/// \todo document
esp_err_t init_rmt(rmt_tx_done_callback_t rmt_cb) {
  rmt_tx_event_callbacks_t cbs{.on_trans_done = rmt_cb};
  return rmt_tx_register_event_callbacks(channel, &cbs, NULL);
}

/// \todo document
esp_err_t init_gpio(gpio_isr_t gpio_isr_handler) {
  ESP_ERROR_CHECK(gpio_isr_handler_add(ack_gpio_num, gpio_isr_handler, NULL));
  return gpio_set_level(enable_gpio_num, 1u);
}

} // namespace

/// \todo document
esp_err_t resume(decup_encoder_config_t const& encoder_config,
                 rmt_tx_done_callback_t rmt_cb,
                 gpio_isr_t gpio_isr_handler) {
  ESP_ERROR_CHECK(init_encoder(encoder_config));
  ESP_ERROR_CHECK(init_rmt(rmt_cb));
  return init_gpio(gpio_isr_handler);
}

} // namespace drv::out::track::decup
