/// TODO
///
/// \file   out/track/utility.hpp
/// \author Vincent Hamp
/// \date   09/07/2023

#pragma once

#include <driver/rmt_encoder.h>

namespace out::track {

/// TODO
inline uint32_t notify_take_sync_reset(TickType_t ticks_to_wait) {
  auto const notification_value{
    ulTaskNotifyTakeIndexed(pdTRUE, default_notify_index, ticks_to_wait)};
  if (notification_value) rmt_sync_reset(synchro);  // This is important :/
  return notification_value;
}

}  // namespace out::track
