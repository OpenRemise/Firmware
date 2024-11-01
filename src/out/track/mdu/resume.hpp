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

/// Initialize peripherals when resuming MDU task
///
/// \file   out/track/mdu/resume.hpp
/// \author Vincent Hamp
/// \date   10/04/2024

#pragma once

#include <driver/gpio.h>
#include <esp_err.h>
#include <rmt_mdu_encoder.h>

namespace out::track::mdu {

esp_err_t init_encoder(mdu_encoder_config_t const& encoder_config);
esp_err_t resume(mdu_encoder_config_t const& encoder_config,
                 gpio_isr_t gpio_isr_handler);

}  // namespace out::track::mdu