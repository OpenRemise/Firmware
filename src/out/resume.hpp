/// TODO
///
/// \file   out/resume.hpp
/// \author Vincent Hamp
/// \date   12/07/2023

#pragma once

#include <driver/gptimer.h>

namespace out {

esp_err_t resume(gptimer_alarm_cb_t gptimer_cb);

}  // namespace out