#include "task_function.hpp"
#include <driver/gpio.h>
#include <mdu/mdu.hpp>
#include "log.h"
#include "resume.hpp"
#include "suspend.hpp"

namespace out::track::mdu {

using namespace ::mdu;

namespace {

// TODO REMOVE
bool d20_state{};
bool d21_state{};

/// TODO
void IRAM_ATTR gpio_isr_handler(void*) {
  gpio_set_level(d21_gpio_num, d21_state = !d21_state);
  uint64_t value{};
  gptimer_get_raw_count(gptimer, &value);

  // Send a notification to the task
  xTaskNotifyIndexedFromISR(task.handle,
                            default_notify_index,
                            static_cast<uint32_t>(value),
                            eSetValueWithoutOverwrite,
                            NULL);
}

/// TODO
esp_err_t transmit_packet(Packet const& packet) {
  static constexpr rmt_transmit_config_t config{.flags = {.eot_level = 1u}};
  return rmt_transmit(channel, encoder, data(packet), size(packet), &config);
}

/// TODO
void alternative_entry() {
  auto const packet{make_busy_packet()};
  auto const then{xTaskGetTickCount() + pdMS_TO_TICKS(200u)};
  while (xTaskGetTickCount() < then) {
    ESP_ERROR_CHECK(transmit_packet(packet));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));
  }
}

/// TODO
constexpr std::pair<uint32_t, uint32_t>
encoder_config2acks(mdu_encoder_config_t const& encoder_config,
                    Packet const& packet) {
  auto const preamble{encoder_config.num_preamble *
                      timings[encoder_config.transfer_rate].one};
  auto const ones{
    std::accumulate(cbegin(packet), cend(packet), 0uz, [](auto a, auto b) {
      return a + std::popcount(b);
    })};
  auto const zeros{size(packet) * CHAR_BIT - ones};

  // Oh fuck... we need to iterate over the data bytes and count zeros and ones

  return {};
}

/// TODO
void loop(mdu_encoder_config_t& encoder_config) {
  // TODO
  alternative_entry();

  for (;;) {
    // BUSY for 2000ms
    {
      auto const then{xTaskGetTickCount() + pdMS_TO_TICKS(2000u)};
      auto const packet{make_busy_packet()};
      while (xTaskGetTickCount() < then) {
        ESP_ERROR_CHECK(transmit_packet(packet));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));
      }
    }

    // Clear whatever notification value there currently is
    xTaskNotifyStateClearIndexed(NULL, default_notify_index);

    // PING
    {
      auto const packet{make_ping_packet(0u, 0u)};
      ESP_ERROR_CHECK(transmit_packet(packet));
      ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0ull));
      gpio_set_level(d20_gpio_num, true);

      // Do this crazy timing calculation stuff here... because here's time

      ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));
      gpio_set_level(d20_gpio_num, false);
      auto const value{
        ulTaskNotifyTakeIndexed(default_notify_index, pdTRUE, 0u)};
      printf("Timer value? %lu\n", value);
    }
  }
}

}  // namespace

/// TODO
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    // TODO num_preamble and num_ackreq should be in NVS
    mdu_encoder_config_t encoder_config{
      .transfer_rate = std::to_underlying(TransferRate::Default),
      .num_preamble = 14u,
      .num_ackreq = 10u,
    };
    ESP_ERROR_CHECK(resume(encoder_config, gpio_isr_handler));
    loop(encoder_config);

    ESP_ERROR_CHECK(suspend());
  }
}

}  // namespace out::track::mdu