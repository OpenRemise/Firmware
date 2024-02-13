#pragma once

#include <driver/gpio.h>
#include <esp_err.h>
#include <rmt_mdu_encoder.h>

namespace out::track::mdu {

esp_err_t init_encoder(mdu_encoder_config_t const& encoder_config);
esp_err_t resume(mdu_encoder_config_t const& encoder_config,
                 gpio_isr_t gpio_isr_handler);

}  // namespace out::track::mdu