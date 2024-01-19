/// Initialize peripherals when resuming DCC task
///
/// \file   out/track/dcc/resume.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <driver/gptimer.h>
#include <rmt_dcc_encoder.h>

namespace out::track::dcc {

esp_err_t resume(dcc_encoder_config_t const& encoder_config,
                 rmt_tx_done_callback_t rmt_cb,
                 gptimer_alarm_cb_t gptimer_cb);

}  // namespace out::track::dcc