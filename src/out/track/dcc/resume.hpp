// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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

}  // namespace out::track::dcc