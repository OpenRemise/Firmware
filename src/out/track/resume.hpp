/// TODO
///
/// \file   out/track/resume.hpp
/// \author Vincent Hamp
/// \date   04/07/2023

#pragma once

#include <driver/gptimer.h>
#include <driver/rmt_encoder.h>

namespace out::track {

esp_err_t resume(rmt_tx_done_callback_t rmt_cb, gptimer_alarm_cb_t gptimer_cb);

}  // namespace out::track