/// Initialize MDU and DCC
///
/// \file   out/track/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include <driver/gpio.h>
#include <driver/rmt_tx.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include "dcc/task_function.hpp"
#include "mdu/task_function.hpp"

namespace out::track {

namespace {

/// TODO
esp_err_t init_gpio() {
  // Open-drain fault pin
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << fault_gpio_num,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
  }

  // Enable pins
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << enable_gpio_num | 1ull << dcc::bidi_en_gpio_num,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(enable_gpio_num, false));
    return gpio_set_level(dcc::bidi_en_gpio_num, false);
  }
}

/// TODO
esp_err_t init_channels() {
  rmt_tx_channel_config_t chan_config{
    .gpio_num = in1_gpio_num,
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = 1'000'000u,
    .mem_block_symbols = 384uz / 4uz,  // 8 channels sharing 384x32 bit RAM
    .trans_queue_depth = 1uz,
    .flags = {
      .invert_out = false,
      .with_dma = false,
      .io_loop_back = false,
      .io_od_mode = false,
    }};

  ESP_ERROR_CHECK(rmt_new_tx_channel(&chan_config, &channels[0uz]));

  chan_config.gpio_num = in2_gpio_num;
  chan_config.flags.invert_out = true;
  ESP_ERROR_CHECK(rmt_new_tx_channel(&chan_config, &channels[1uz]));

  std::ranges::for_each(channels,
                        [](auto&& ch) { ESP_ERROR_CHECK(rmt_enable(ch)); });

  return ESP_OK;
}

/// TODO
esp_err_t init_sync_manager() {
  static constexpr rmt_sync_manager_config_t synchro_config{
    .tx_channel_array = data(channels),
    .array_size = size(channels),
  };

  ESP_ERROR_CHECK(rmt_new_sync_manager(&synchro_config, &synchro));

  return ESP_OK;
}

}  // namespace

/// TODO
esp_err_t init() {
  rx_queue.handle = xQueueCreate(rx_queue.size, sizeof(RxQueue::value_type));

  ESP_ERROR_CHECK(init_gpio());
  ESP_ERROR_CHECK(init_channels());
  ESP_ERROR_CHECK(init_sync_manager());

  xTaskCreatePinnedToCore(dcc::task_function,
                          dcc::task.name,
                          dcc::task.stack_depth,
                          NULL,
                          dcc::task.priority,
                          &dcc::task.handle,
                          1);
  xTaskCreatePinnedToCore(mdu::task_function,
                          mdu::task.name,
                          mdu::task.stack_depth,
                          NULL,
                          mdu::task.priority,
                          &mdu::task.handle,
                          1);

  return ESP_OK;
}

}  // namespace out::track
