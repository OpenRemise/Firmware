#include "resume.hpp"
#include <rmt_mdu_encoder.h>
#include "analog/init.hpp"

namespace out::track::mdu {

///
esp_err_t resume(mdu_encoder_config_t const& encoder_config,
                 rmt_tx_done_callback_t rmt_cb) {
  return ESP_OK;
}

}  // namespace out::track::mdu