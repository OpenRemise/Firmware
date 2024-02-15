/// TODO
///
/// \file   out/track/suspend.cpp
/// \author Vincent Hamp
/// \date   04/07/2023

#include "suspend.hpp"
#include <driver/gpio.h>
#include "../suspend.hpp"

namespace out::track {

namespace {

/// TODO
esp_err_t deinit_gpio() { return gpio_set_level(enable_gpio_num, 0u); }

/// TODO
esp_err_t deinit_rmt() {
  rmt_tx_event_callbacks_t cbs{};
  return rmt_tx_register_event_callbacks(channel, &cbs, NULL);
}

}  // namespace

/// TODO
esp_err_t deinit_encoder() {
  ESP_ERROR_CHECK(rmt_del_encoder(encoder));
  encoder = NULL;
  return ESP_OK;
}

/// TODO
esp_err_t suspend() {
  ESP_ERROR_CHECK(deinit_gpio());
  ESP_ERROR_CHECK(deinit_rmt());
  ESP_ERROR_CHECK(deinit_encoder());
  return out::suspend();
}

}  // namespace out::track