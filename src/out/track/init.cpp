// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize MDU and DCC
///
/// \file   out/track/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include <driver/gpio.h>
#include <driver/rmt_tx.h>
#include <array>
#include <cstring>
#include <limits>
#include "dcc/task_function.hpp"
#include "decup/task_function.hpp"
#include "log.h"
#include "mdu/task_function.hpp"
#include "utility.hpp"

namespace out::track {

namespace {

/// \todo document this should never happen
void IRAM_ATTR nfault_isr_handler(void*) {
  DRAM_LOGE("nFAULT");
  bug_led(true);
}

/// \todo document RMT pin no longer tristate after that
esp_err_t init_channel() {
  static constexpr rmt_tx_channel_config_t chan_config{
    .gpio_num = left_gpio_num,
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = 1'000'000u,
    .mem_block_symbols =
      SOC_RMT_CHANNELS_PER_GROUP *
      SOC_RMT_MEM_WORDS_PER_CHANNEL,  // 8 channels sharing 384x32 bit RAM
    .trans_queue_depth = trans_queue_depth,
    .intr_priority = 3,
    .flags = {
      .invert_out = false,
      .with_dma = false,
      .io_loop_back = false,
      .io_od_mode = false,
    }};
  ESP_ERROR_CHECK(rmt_new_tx_channel(&chan_config, &channel));
  return rmt_enable(channel);
}

/// \todo document
esp_err_t init_gpio() {
  // Outputs
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << isel0_gpio_num | 1ull << isel1_gpio_num |
                      1ull << nsleep_gpio_num |
                      1ull << right_force_low_gpio_num |
                      1ull << enable_gpio_num | 1ull << dcc::bidi_en_gpio_num,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(right_force_low_gpio_num, 1u));
    ESP_ERROR_CHECK(gpio_set_level(enable_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(dcc::bidi_en_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(nsleep_gpio_num, 1u));
  }

  // Wait for device to wake up
  // (otherwise nFAULT would immediately trigger an interrupt)
  vTaskDelay(pdMS_TO_TICKS(100u));

  // Install a global ISR handler service to allow adding callbacks for
  // individual GPIO pins via gpio_isr_handler_add
  ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL2));

  // Inputs
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << nfault_gpio_num | 1ull << ack_gpio_num,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_NEGEDGE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
  }

  // Add an ISR handler for nFAULT
  return gpio_isr_handler_add(nfault_gpio_num, nfault_isr_handler, NULL);
}

}  // namespace

/// \todo document
esp_err_t init(BaseType_t xCoreID) {
  rx_queue.handle = xQueueCreate(rx_queue.size, sizeof(RxQueue::value_type));

  ESP_ERROR_CHECK(init_channel());
  ESP_ERROR_CHECK(init_gpio());

  if (!xTaskCreatePinnedToCore(dcc::task_function,
                               dcc::task.name,
                               dcc::task.stack_size,
                               NULL,
                               dcc::task.priority,
                               &dcc::task.handle,
                               xCoreID))
    assert(false);
  if (!xTaskCreatePinnedToCore(decup::task_function,
                               decup::task.name,
                               decup::task.stack_size,
                               NULL,
                               decup::task.priority,
                               &decup::task.handle,
                               xCoreID))
    assert(false);
  if (!xTaskCreatePinnedToCore(mdu::task_function,
                               mdu::task.name,
                               mdu::task.stack_size,
                               NULL,
                               mdu::task.priority,
                               &mdu::task.handle,
                               xCoreID))
    assert(false);

  return ESP_OK;
}

}  // namespace out::track
