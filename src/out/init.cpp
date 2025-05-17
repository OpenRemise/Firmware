// Copyright (C) 2025 Vincent Hamp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
    .resolution_hz = 1'000'000u, // 1 MHz
    .intr_priority = 2           // Priority 3 is taken by RMT!
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

} // namespace out
