/// TODO
///
/// \file   out/resume.cpp
/// \author Vincent Hamp
/// \date   12/07/2023

#include "resume.hpp"
#include <driver/gpio.h>

namespace out {

namespace {

/// TODO
esp_err_t init_alarm(gptimer_alarm_cb_t gptimer_cb) {
  gptimer_event_callbacks_t cbs{.on_alarm = gptimer_cb};
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
  ESP_ERROR_CHECK(gptimer_enable(gptimer));
  return gptimer_start(gptimer);
}

}  // namespace

/// TODO
esp_err_t resume(gptimer_alarm_cb_t gptimer_cb) {
  return init_alarm(gptimer_cb);
}

}  // namespace out