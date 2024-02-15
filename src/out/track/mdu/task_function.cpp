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
esp_err_t transmit_packet_wait_all_done(Packet const& packet) {
  static constexpr rmt_transmit_config_t config{.flags = {.eot_level = 1u}};
  ESP_ERROR_CHECK(
    rmt_transmit(channel, encoder, data(packet), size(packet), &config));

  // Start timer
  ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0ull));

  // Clear any stored counts
  xTaskNotifyStateClearIndexed(NULL, default_notify_index);

  // Wait
  return rmt_tx_wait_all_done(channel, -1);
}

/// TODO
void alternative_entry() {
  auto const packet{make_busy_packet()};
  auto const then{xTaskGetTickCount() + pdMS_TO_TICKS(200u)};
  while (xTaskGetTickCount() < then)
    ESP_ERROR_CHECK(transmit_packet_wait_all_done(packet));
}

/// TODO
std::pair<int32_t, int32_t>
packet2ack_counts(mdu_encoder_config_t const& encoder_config,
                  Packet const& packet) {
  // Preamble
  auto count{static_cast<int32_t>(encoder_config.num_preamble *
                                  timings[encoder_config.transfer_rate].one)};

  // Start
  count += size(packet) * timings[encoder_config.transfer_rate].zero;

  // Data
  auto const ones{
    std::accumulate(cbegin(packet), cend(packet), 0uz, [](auto a, auto b) {
      return a + std::popcount(b);
    })};
  auto const zeros{size(packet) * CHAR_BIT - ones};
  count += ones * timings[encoder_config.transfer_rate].one +   // Ones
           zeros * timings[encoder_config.transfer_rate].zero;  // Zeros

  // End
  count += timings[encoder_config.transfer_rate].one;

  return {count + 2u * timings[encoder_config.transfer_rate].ackreq,
          count + 6u * timings[encoder_config.transfer_rate].ackreq};
}

/// TODO
void receive_acks(mdu_encoder_config_t const& encoder_config,
                  Packet const& packet) {
  // Get timer count
  auto const count{static_cast<int32_t>(
    ulTaskNotifyTakeIndexed(default_notify_index, pdTRUE, 0u))};
  if (!count) return;

  //
  auto const ack_counts{packet2ack_counts(encoder_config, packet)};
  std::pair const ack_diffs{std::abs(ack_counts.first - count),
                            std::abs(ack_counts.second - count)};
  auto const ack1{ack_diffs.first < ack_diffs.second};
  std::array<uint8_t, 2uz> const acks{ack1, !ack1};
  printf("acks %d / %d\n", acks[0uz], acks[1uz]);
  xMessageBufferSend(out::rx_message_buffer.handle,
                     data(acks),
                     size(acks),
                     pdMS_TO_TICKS(task.timeout));
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
      while (xTaskGetTickCount() < then)
        ESP_ERROR_CHECK(transmit_packet_wait_all_done(packet));
    }

    // PING (ACK in CH2)
    {
      auto const packet{make_ping_packet(0u, 0u)};
      gpio_set_level(d20_gpio_num, true);
      ESP_ERROR_CHECK(transmit_packet_wait_all_done(packet));
      gpio_set_level(d20_gpio_num, false);
      receive_acks(encoder_config, packet);
    }

    // BUSY for 2000ms
    {
      auto const then{xTaskGetTickCount() + pdMS_TO_TICKS(2000u)};
      auto const packet{make_busy_packet()};
      while (xTaskGetTickCount() < then)
        ESP_ERROR_CHECK(transmit_packet_wait_all_done(packet));
    }

    // PING with wrong CRC (ACK in CH1)
    {
      auto packet{make_ping_packet(0u, 0u)};
      packet[1uz]++;
      gpio_set_level(d20_gpio_num, true);
      ESP_ERROR_CHECK(transmit_packet_wait_all_done(packet));
      gpio_set_level(d20_gpio_num, false);
      receive_acks(encoder_config, packet);
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