/// TODO
///
/// \file   out/suspend.cpp
/// \author Vincent Hamp
/// \date   23/04/2023

#include "suspend.hpp"
#include <driver/gptimer.h>
#include "log.h"

namespace out {

namespace {

/// TODO
esp_err_t deinit_alarm() {
  gptimer_stop(gptimer);
  ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0ull));
  ESP_ERROR_CHECK(gptimer_disable(gptimer));
  gptimer_event_callbacks_t cbs{};
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
  return gptimer_set_alarm_action(gptimer, NULL);
}

/// TODO
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

}  // namespace

/// TODO
esp_err_t suspend() {
  ESP_ERROR_CHECK(deinit_alarm());
  reset_queue_and_message_buffers();
  mode.store(Mode::Suspended);
  return ESP_OK;
}

}  // namespace out