#include "task_function.hpp"
#include <driver/gpio.h>
#include <mdu/mdu.hpp>
#include "../current_limit.hpp"
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
esp_err_t transmit_packet_wait_all_done_for(Packet const& packet, uint32_t ms) {
  auto const then{xTaskGetTickCount() + pdMS_TO_TICKS(ms)};
  while (xTaskGetTickCount() < then)
    ESP_ERROR_CHECK(transmit_packet_wait_all_done(packet));
  return ESP_OK;
}

/// TODO
std::optional<Packet> receive_packet() {
  Packet packet;
  //
  if (auto const bytes_received{xMessageBufferReceive(
        tx_message_buffer.front_handle, data(packet), packet.max_size(), 0u)}) {
    packet.resize(bytes_received);
    return packet;
  }
  //
  else
    return std::nullopt;
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
std::array<uint8_t, 2uz>
receive_acks(mdu_encoder_config_t const& encoder_config, Packet const& packet) {
  // Get timer count
  auto const count{static_cast<int32_t>(
    ulTaskNotifyTakeIndexed(default_notify_index, pdTRUE, 0u))};
  std::array<uint8_t, 2uz> acks{nak, nak};

  // ACK
  if (count) {
    auto const ack_counts{packet2ack_counts(encoder_config, packet)};
    std::pair const ack_diffs{std::abs(ack_counts.first - count),
                              std::abs(ack_counts.second - count)};
    if (ack_diffs.first < ack_diffs.second) acks[0uz] = ack;
    else acks[1uz] = ack;
  }

  return acks;
}

/// TODO
esp_err_t transmit_acks(std::array<uint8_t, 2uz> acks) {
  return xMessageBufferSend(out::rx_message_buffer.handle,
                            data(acks),
                            size(acks),
                            pdMS_TO_TICKS(task.timeout)) == size(acks)
           ? ESP_OK
           : ESP_FAIL;
}

/// TODO
esp_err_t config_transfer_rate(mdu_encoder_config_t& encoder_config,
                               uint8_t transfer_rate,
                               std::array<uint8_t, 2uz> const& acks) {
  assert(transfer_rate <= std::to_underlying(TransferRate::Default));
  ESP_ERROR_CHECK(deinit_encoder());
  encoder_config.transfer_rate = acks[0uz] == ack || acks[1uz] == ack
                                   ? std::to_underlying(TransferRate::Fallback)
                                   : transfer_rate;
  ESP_ERROR_CHECK(init_encoder(encoder_config));
  return acks[0uz] == ack ? ESP_ERR_INVALID_CRC : ESP_OK;
}

/// TODO
esp_err_t loop(mdu_encoder_config_t& encoder_config) {
  auto const busy_packet{make_busy_packet()};
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};

  // Alternative entry
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_4100mA));
  ESP_ERROR_CHECK(transmit_packet_wait_all_done_for(busy_packet, 200u));
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_500mA));

  for (;;) {
    auto const packet{receive_packet()};

    // Return on timeout
    if (auto const now{xTaskGetTickCount()}; now >= then)
      return rmt_tx_wait_all_done(channel, -1);
    // In case we got data, reset timeout
    else if (packet) then = now + pdMS_TO_TICKS(task.timeout);
    // We got no data, transmit busy packet
    else {
      ESP_ERROR_CHECK(transmit_packet_wait_all_done(busy_packet));
      auto const acks{receive_acks(encoder_config, *packet)};
      if (acks[0uz] == ack || acks[1uz] == ack) vTaskDelay(pdMS_TO_TICKS(20u));
      continue;
    }

    ESP_ERROR_CHECK(transmit_packet_wait_all_done(*packet));
    auto const acks{receive_acks(encoder_config, *packet)};
    ESP_ERROR_CHECK(transmit_acks(acks));

    // Transfer rate packet
    if (auto const cmd{packet2command(*packet)};
        cmd == Command::ConfigTransferRate)
      config_transfer_rate(encoder_config, (*packet)[4uz], acks);
    // Exit
    else if (cmd == Command::ZppExitReset || cmd == Command::ZsuCrc32ResultExit)
      return transmit_packet_wait_all_done_for(busy_packet, 1000u);
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
    ESP_ERROR_CHECK(loop(encoder_config));

    ESP_ERROR_CHECK(suspend());
  }
}

}  // namespace out::track::mdu