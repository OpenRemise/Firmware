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

  xMessageBufferSend(out::rx_message_buffer.handle,
                     data(acks),
                     size(acks),
                     pdMS_TO_TICKS(task.timeout));
  return acks;
}

/// TODO
esp_err_t config_transfer_rate(mdu_encoder_config_t& encoder_config,
                               uint8_t transfer_rate,
                               std::array<uint8_t, 2uz> const& acks) {
  if (acks[0uz] == ack) return ESP_ERR_INVALID_CRC;
  assert(transfer_rate <= std::to_underlying(TransferRate::Default));
  ESP_ERROR_CHECK(deinit_encoder());
  encoder_config.transfer_rate = acks[1uz] == ack
                                   ? std::to_underlying(TransferRate::Fallback)
                                   : transfer_rate;
  return init_encoder(encoder_config);
}

/// TODO
void loop(mdu_encoder_config_t& encoder_config) {
  auto const busy_packet{make_busy_packet()};
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};

  // Alternative entry
  ESP_ERROR_CHECK(transmit_packet_wait_all_done_for(busy_packet, 200u));

  for (;;) {
    auto const packet{receive_packet()};

    // Return on timeout
    if (auto const now{xTaskGetTickCount()}; now >= then) return;
    // In case we got data, reset timeout
    else if (packet) then = now + pdMS_TO_TICKS(task.timeout);
    // We got no data, transmit busy packet
    else {
      ESP_ERROR_CHECK(transmit_packet_wait_all_done(busy_packet));
      continue;
    }

    ESP_ERROR_CHECK(transmit_packet_wait_all_done(*packet));
    auto const acks{receive_acks(encoder_config, *packet)};

    // Transfer rate packet
    if (auto const cmd{packet2command(*packet)};
        cmd == Command::ConfigTransferRate)
      config_transfer_rate(encoder_config, (*packet)[4uz], acks);
    // Exit
    else if (cmd == Command::ZppExitReset ||
             cmd == Command::ZsuCrc32ResultExit) {
      ESP_ERROR_CHECK(transmit_packet_wait_all_done_for(busy_packet, 1000u));
      return;
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