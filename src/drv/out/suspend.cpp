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
#include "utility.hpp"

namespace drv::out {

namespace {

/// \todo document
void reset_queue_and_message_buffers() {
  xQueueReset(track::rx_queue.handle);
  reset_rx_message_buffer_blocking();
  reset_tx_message_buffer_front_blocking();
  reset_tx_message_buffer_back_blocking();
}

} // namespace

/// \todo document
esp_err_t suspend() {
  reset_queue_and_message_buffers();
  if (!(state.load() & State::ShortCircuit)) state.store(State::Suspended);
  return ESP_OK;
}

} // namespace drv::out
