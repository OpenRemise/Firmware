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

/// \todo document
///
/// \file   drv/out/suspend.cpp
/// \author Vincent Hamp
/// \date   23/04/2023

#include "suspend.hpp"
#include "log.h"

namespace drv::out {

namespace {

/// \todo document
void reset_queue_and_message_buffers() {
  xQueueReset(track::rx_queue.handle);
  while (!xMessageBufferReset(rx_message_buffer.handle)) {
    LOGW("Can't reset drv::out::rx_message_buffer");
    vTaskDelay(pdMS_TO_TICKS(20u));
  }
  // Don't short circuit here!
  while (!xMessageBufferReset(tx_message_buffer.front_handle) |
         !xMessageBufferReset(tx_message_buffer.back_handle)) {
    LOGW("Can't reset drv::out::tx_message_buffer");
    vTaskDelay(pdMS_TO_TICKS(20u));
  }
}

} // namespace

/// \todo document
esp_err_t suspend() {
  reset_queue_and_message_buffers();
  if (!(state.load() & State::ShortCircuit)) state.store(State::Suspended);
  return ESP_OK;
}

} // namespace drv::out
