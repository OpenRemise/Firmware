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

/// Initialize peripherals when resuming DCC task
///
/// \file   out/track/dcc/resume.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <driver/gptimer.h>
#include <rmt_dcc_encoder.h>

namespace out::track::dcc {

esp_err_t init_encoder(dcc_encoder_config_t const& encoder_config);
esp_err_t init_rmt(rmt_tx_done_callback_t rmt_cb);
esp_err_t init_alarm(gptimer_alarm_cb_t gptimer_cb);
esp_err_t init_bidi();
esp_err_t init_gpio();
esp_err_t resume(dcc_encoder_config_t const& encoder_config,
                 rmt_tx_done_callback_t rmt_cb,
                 gptimer_alarm_cb_t gptimer_cb);

} // namespace out::track::dcc
