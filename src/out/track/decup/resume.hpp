// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize peripherals when resuming DECUP task
///
/// \file   out/track/decup/resume.hpp
/// \author Vincent Hamp
/// \date   14/08/2024

#pragma once

#include <driver/gpio.h>
#include <esp_err.h>
#include "rmt_decup_encoder.h"

namespace out::track::decup {

esp_err_t init_encoder(decup_encoder_config_t const& encoder_config);
esp_err_t resume(decup_encoder_config_t const& encoder_config,
                 gpio_isr_t gpio_isr_handler);

}  // namespace out::track::decup