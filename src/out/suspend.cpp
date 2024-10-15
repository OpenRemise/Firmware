// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// \todo document
///
/// \file   out/suspend.cpp
/// \author Vincent Hamp
/// \date   23/04/2023

#include "suspend.hpp"
#include "log.h"

namespace out {

/// \todo document
void reset_queue_and_message_buffers() {
  xQueueReset(track::rx_queue.handle);
  while (!xMessageBufferReset(rx_message_buffer.handle)) {
    LOGW("Can't reset out::rx_message_buffer");
    vTaskDelay(pdMS_TO_TICKS(20u));
  }
  // Don't short circuit here!
  while (!xMessageBufferReset(tx_message_buffer.front_handle) |
         !xMessageBufferReset(tx_message_buffer.back_handle)) {
    LOGW("Can't reset out::tx_message_buffer");
    vTaskDelay(pdMS_TO_TICKS(20u));
  }
}

/// \todo document
esp_err_t suspend() {
  reset_queue_and_message_buffers();
  if (!(state.load() & State::ShortCircuit)) state.store(State::Suspended);
  return ESP_OK;
}

}  // namespace out