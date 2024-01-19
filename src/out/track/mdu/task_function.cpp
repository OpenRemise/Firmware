#include "task_function.hpp"
#include <driver/gpio.h>
#include <algorithm>
#include <array>
#include <mdu/mdu.hpp>
#include "../utility.hpp"
#include "analog/convert.hpp"
#include "log.h"
#include "resume.hpp"
#include "suspend.hpp"

namespace out::track::mdu {

using namespace analog;
using namespace ::mdu;

namespace {

constexpr std::array<rmt_transmit_config_t, 2uz> tx_configs{};

/// TODO
bool IRAM_ATTR rmt_callback(rmt_channel_handle_t,
                            rmt_tx_done_event_data_t const*,
                            void*) {
  BaseType_t high_task_awoken{pdFALSE};
  vTaskNotifyGiveIndexedFromISR(
    task.handle, default_notify_index, &high_task_awoken);
  return pdFALSE;
}

/// TODO
std::optional<Packet> receive_packet() {
  Packet packet;
  //
  if (auto const bytes_received{
        xMessageBufferReceive(tx_message_buffer.front_handle,
                              data(packet),
                              packet.max_size(),
                              pdMS_TO_TICKS(task.timeout))}) {
    packet.resize(bytes_received);
    return packet;
  }
  //
  else
    return std::nullopt;
}

/// TODO
esp_err_t transmit_packet(Packet const& packet) {
  return rmt_transmit(channels[0uz],
                      encoders[0uz],
                      data(packet),
                      size(packet),
                      &tx_configs[0uz]) ||
         rmt_transmit(channels[1uz],
                      encoders[1uz],
                      data(packet),
                      size(packet),
                      &tx_configs[1uz]);
}

/// Send MDU for at least 200ms
void alternative_entry() {
  auto const packet{make_busy_packet()};
  auto const then{xTaskGetTickCount() + pdMS_TO_TICKS(200u)};
  while (xTaskGetTickCount() < then) {
    ESP_ERROR_CHECK(transmit_packet(packet));
    notify_take_sync_reset(portMAX_DELAY);
  }
}

/// TODO
size_t measure_currents(CurrentMeasurement* currents) {
  size_t count{};

  // Measure as long as packet gets transmitted
  while (!notify_take_sync_reset(0u))
    if (xQueueReceive(currents_queue.handle, &currents[count], 0u))
      count += std::tuple_size_v<CurrentsQueue::value_type>;
  uint64_t before{};
  ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &before));

  // Get one last...
  if (xQueueReceive(currents_queue.handle, &currents[count], portMAX_DELAY))
    count += std::tuple_size_v<CurrentsQueue::value_type>;
  uint64_t after{};
  ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &after));
  auto const dt{static_cast<uint32_t>(after - before)};

  // Subtract time between RMT end and last ADC queue access from count
  count -= dt / static_cast<uint32_t>(analog::sample_time_per_channel_us);

  return count;
}

/// TODO
uint8_t high_current_between(CurrentMeasurement* first,
                             CurrentMeasurement* last) {
  static auto thr{mA2measurement(ack_threshold)};
  auto const count_above_thr{std::count_if(
    first, last, [](CurrentMeasurement const& meas) { return meas > thr; })};
  return count_above_thr >= 3 ? ack : nak;
}

/// TODO
std::array<uint8_t, 2uz>
measure_acks(mdu_encoder_config_t const& encoder_config,
             CurrentMeasurement* currents) {
  auto const count{measure_currents(currents)};
  auto const [ack1_first, ack1_second]{ack1_indices(encoder_config, count)};
  assert(ack1_first < count && ack1_second <= count);
  auto const ack1{
    high_current_between(&currents[ack1_first], &currents[ack1_second])};
  auto const [ack2_first, ack2_second]{ack2_indices(encoder_config, count)};
  assert(ack2_first < count && ack2_second <= count);
  auto const ack2{
    high_current_between(&currents[ack2_first], &currents[ack2_second])};
  return {ack1, ack2};
}

/// TODO
void transmit_acks(std::span<uint8_t const> acks) {
  xMessageBufferSend(out::rx_message_buffer.handle,
                     data(acks),
                     size(acks),
                     pdMS_TO_TICKS(task.timeout));
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
  auto currents{std::make_unique<CurrentMeasurement[]>(64uz * 1024uz)};

  // TODO
  alternative_entry();

  for (;;) {
    auto packet{receive_packet()};

    // Return on timeout
    if (!packet) return;

    ESP_ERROR_CHECK(transmit_packet(*packet));
    auto const acks{measure_acks(encoder_config, currents.get())};
    transmit_acks(acks);

    // Transfer rate packet
    if (auto const cmd{packet2command(*packet)};
        cmd == Command::ConfigTransferRate)
      config_transfer_rate(encoder_config, (*packet)[4uz], acks);
    // Exit
    else if (cmd == Command::ZppExitReset ||
             cmd == Command::ZsuCrc32ResultExit) {
      vTaskDelay(pdMS_TO_TICKS(1000u));
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

    ESP_ERROR_CHECK(resume(encoder_config, rmt_callback));
    loop(encoder_config);
    ESP_ERROR_CHECK(suspend());
  }
}

}  // namespace out::track::mdu