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

/// Initialize MDU and DCC
///
/// \file   out/track/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include <driver/gpio.h>
#include <driver/gpio_filter.h>
#include <driver/rmt_tx.h>
#include <array>
#include <cstring>
#include <limits>
#include "dcc/task_function.hpp"
#include "decup/task_function.hpp"
#include "led/bug.hpp"
#include "log.h"
#include "mdu/task_function.hpp"
#include "utility.hpp"

namespace out::track {

namespace {

/// \todo document this should never happen
void IRAM_ATTR nfault_isr_handler(void*) {
  DRAM_LOGE("nFAULT");
  led::bug(true);
}

/// \todo document RMT pin no longer tristate after that
esp_err_t init_channel() {
  static constexpr rmt_tx_channel_config_t chan_config{
    .gpio_num = p_gpio_num,
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = 1'000'000u,
    .mem_block_symbols =
      SOC_RMT_CHANNELS_PER_GROUP *
      SOC_RMT_MEM_WORDS_PER_CHANNEL, // 8 channels sharing 384x32 bit RAM
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
  // Inputs and outputs
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << ilim0_gpio_num | 1ull << ilim1_gpio_num |
                      1ull << enable_gpio_num,
      .mode = GPIO_MODE_INPUT_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(enable_gpio_num, 0u));
  }

  // Outputs
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << nsleep_gpio_num | 1ull << n_force_low_gpio_num |
                      1ull << dcc::bidi_en_gpio_num,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(n_force_low_gpio_num, 1u));
    ESP_ERROR_CHECK(gpio_set_level(dcc::bidi_en_gpio_num, 0u));
    ESP_ERROR_CHECK(gpio_set_level(nsleep_gpio_num, 1u));
  }

  // Wait for device to wake up
  // (otherwise nFAULT would immediately trigger an interrupt)
  vTaskDelay(pdMS_TO_TICKS(100u));

  // Install a global ISR handler service to allow adding callbacks for
  // individual GPIO pins via gpio_isr_handler_add
  ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL2));

  //
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << nfault_gpio_num,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_NEGEDGE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
  }

  //
  {
    static constexpr gpio_config_t io_conf{.pin_bit_mask = 1ull << ack_gpio_num,
                                           .mode = GPIO_MODE_INPUT,
                                           .pull_up_en = GPIO_PULLUP_ENABLE,
                                           .pull_down_en =
                                             GPIO_PULLDOWN_DISABLE,
                                           .intr_type = GPIO_INTR_NEGEDGE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_glitch_filter_handle_t filter;
    static constexpr gpio_pin_glitch_filter_config_t config{
      .clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT, .gpio_num = ack_gpio_num};
    ESP_ERROR_CHECK(gpio_new_pin_glitch_filter(&config, &filter));
    ESP_ERROR_CHECK(gpio_glitch_filter_enable(filter));
  }

  // Add an ISR handler for nFAULT
  return gpio_isr_handler_add(nfault_gpio_num, nfault_isr_handler, NULL);
}

} // namespace

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

} // namespace out::track
