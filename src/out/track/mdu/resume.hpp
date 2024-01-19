#pragma once

#include <esp_err.h>
#include <rmt_mdu_encoder.h>

namespace out::track::mdu {

esp_err_t init_encoder(mdu_encoder_config_t const& encoder_config);
esp_err_t resume(mdu_encoder_config_t const& encoder_config,
                 rmt_tx_done_callback_t rmt_cb);

}  // namespace out::track::mdu