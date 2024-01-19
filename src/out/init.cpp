/// Initialize low level layer (ADC, track and ZUSI)
///
/// \file   out/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include "track/init.hpp"
#include "zusi/init.hpp"

namespace out {

/// TODO
esp_err_t init_gptimer() {
  static constexpr gptimer_config_t timer_config{
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1'000'000u  // 1 MHz
  };

  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

  return ESP_OK;
}

/// TODO
esp_err_t init() {
  rx_message_buffer.handle = xMessageBufferCreate(rx_message_buffer.size);
  tx_message_buffer.front_handle = xMessageBufferCreate(tx_message_buffer.size);
  tx_message_buffer.back_handle = xMessageBufferCreate(tx_message_buffer.size);

  ESP_ERROR_CHECK(init_gptimer());

  //
  ESP_ERROR_CHECK(track::init());
  ESP_ERROR_CHECK(zusi::init());
  return ESP_OK;
}

}  // namespace out