// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize low level layer (ADC, track and ZUSI)
///
/// \file   out/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include "track/init.hpp"
#include "zusi/init.hpp"

namespace out {

/// \todo document
esp_err_t init_gptimer() {
  static constexpr gptimer_config_t timer_config{
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1'000'000u,  // 1 MHz
    .intr_priority = 2            // Priority 3 is taken by RMT!
  };
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

  // Install interrupt (nullptr argument doesn't matter)
  gptimer_event_callbacks_t cbs{.on_alarm = nullptr};
  return gptimer_register_event_callbacks(gptimer, &cbs, NULL);
}

/// \todo document
esp_err_t init(BaseType_t xCoreID) {
  rx_message_buffer.handle = xMessageBufferCreate(rx_message_buffer.size);
  tx_message_buffer.front_handle = xMessageBufferCreate(tx_message_buffer.size);
  tx_message_buffer.back_handle = xMessageBufferCreate(tx_message_buffer.size);

  ESP_ERROR_CHECK(init_gptimer());

  //
  ESP_ERROR_CHECK(track::init(xCoreID));
  ESP_ERROR_CHECK(zusi::init(xCoreID));

  return ESP_OK;
}

}  // namespace out