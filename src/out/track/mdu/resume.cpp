#include "resume.hpp"
#include <rmt_mdu_encoder.h>
#include "../resume.hpp"

namespace out::track::mdu {

namespace {

///
esp_err_t init_gpio(gpio_isr_t gpio_isr_handler) {
  return gpio_isr_handler_add(ack_gpio_num, gpio_isr_handler, NULL);
}

}  // namespace

///
esp_err_t init_encoder(mdu_encoder_config_t const& encoder_config) {
  assert(!encoder);
  return rmt_new_mdu_encoder(&encoder_config, &encoder);
}

///
esp_err_t resume(mdu_encoder_config_t const& encoder_config,
                 gpio_isr_t gpio_isr_handler) {
  ESP_ERROR_CHECK(init_gpio(gpio_isr_handler));
  ESP_ERROR_CHECK(init_encoder(encoder_config));
  return track::resume(nullptr, nullptr);
}

}  // namespace out::track::mdu