#include "resume.hpp"
#include <rmt_mdu_encoder.h>
#include <algorithm>
#include "../resume.hpp"
#include "analog/init.hpp"

namespace out::track::mdu {

///
esp_err_t init_encoder(mdu_encoder_config_t const& encoder_config) {
  std::ranges::for_each(encoders, [&encoder_config](auto&& e) {
    assert(!e);
    ESP_ERROR_CHECK(rmt_new_mdu_encoder(&encoder_config, &e));
  });
  return ESP_OK;
}

///
esp_err_t resume(mdu_encoder_config_t const& encoder_config,
                 rmt_tx_done_callback_t rmt_cb) {
  ESP_ERROR_CHECK(analog::init(analog::mdu_sample_freq_hz));
  ESP_ERROR_CHECK(init_encoder(encoder_config));
  return track::resume(rmt_cb, nullptr);
}

}  // namespace out::track::mdu