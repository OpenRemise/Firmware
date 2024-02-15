/// TODO
///
/// \file   out/track/resume.cpp
/// \author Vincent Hamp
/// \date   04/07/2023

#include "resume.hpp"
#include <driver/gpio.h>
#include "../resume.hpp"

namespace out::track {

/// TODO
esp_err_t init_rmt(rmt_tx_done_callback_t rmt_cb) {
  rmt_tx_event_callbacks_t cbs{.on_trans_done = rmt_cb};
  return rmt_tx_register_event_callbacks(channel, &cbs, NULL);
}

/// TODO
esp_err_t init_gpio() { return gpio_set_level(enable_gpio_num, 1u); }

/// TODO
esp_err_t resume(rmt_tx_done_callback_t rmt_cb, gptimer_alarm_cb_t gptimer_cb) {
  ESP_ERROR_CHECK(out::resume(gptimer_cb));
  ESP_ERROR_CHECK(init_rmt(rmt_cb));
  return init_gpio();
}

}  // namespace out::track