// Copyright (C) 2024 Vincent Hamp
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

/// DECUP task function
///
/// \file   out/track/decup/task_function.cpp
/// \author Vincent Hamp
/// \date   14/08/2024

#include "task_function.hpp"
#include <driver/gpio.h>
#include <decup/decup.hpp>
#include <ulf/decup_ein.hpp>
#include "../current_limit.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "resume.hpp"
#include "suspend.hpp"
#include "utility.hpp"

namespace out::track::decup {

using namespace ::decup;
using ::ulf::decup_ein::ack, ::ulf::decup_ein::nak;

namespace {

bool gpio1_state{};
bool gpio2_state{};

/// \todo document
void IRAM_ATTR ack_isr_handler(void*) {
  uint64_t u64{};
  gptimer_get_raw_count(gptimer, &u64);
  auto const u32{static_cast<uint32_t>(u64)};
  xMessageBufferSendFromISR(
    tx_message_buffer.back_handle, &u32, sizeof(u32), NULL);

  gpio_set_level(GPIO_NUM_1, gpio1_state = !gpio1_state);
}

/// \todo document
std::optional<Packet> receive_packet() {
  Packet packet;
  //
  if (auto const bytes_received{
        xMessageBufferReceive(tx_message_buffer.front_handle,
                              data(packet),
                              packet.max_size(),
                              pdMS_TO_TICKS(http_receive_timeout2ms()))}) {
    packet.resize(bytes_received);
    return packet;
  }
  //
  else
    return std::nullopt;
}

/// \todo document
esp_err_t transmit_packet_blocking(Packet const& packet) {
  static constexpr rmt_transmit_config_t config{.flags = {.eot_level = 1u}};
  ESP_ERROR_CHECK(
    rmt_transmit(channel, encoder, data(packet), size(packet), &config));

  // Start timer
  ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0ull));

  // Wait
  ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));

  // Clear any stored timer values
  return xMessageBufferReset(tx_message_buffer.back_handle) ? ESP_OK : ESP_FAIL;
}

/// \todo document
/// MX645 @ 1.3A
/// 18V 154-156 360-362
/// 16V 152-153 357-359
/// 14V 149-150 354-356
/// 12V 145-146 345-347
/// 10V 137-138 330-332
///
/// MX699 @ 1.3A
/// 18V x
/// 16V 214-215 422-427
/// 14V 235-237 414-416
/// 12V 143-146 364-366
/// 10V 163-164 278-280
///
/// MX699 @ 2.7A
/// 18V 267-267 314-315
/// 10V x (Doppelpuls bricht nicht mehr ein)
uint32_t measure_pulse_width(uint32_t us) {
  auto then{esp_timer_get_time() + us};

  // Get reference value
  uint32_t begin{};
  while (esp_timer_get_time() < then)
    if (xMessageBufferReceive(
          tx_message_buffer.back_handle, &begin, sizeof(begin), 0u))
      break;

  // Got nothing
  if (!begin) return 0u;

  // As long as end keeps changing within 500us loop
  uint32_t end{begin};
  while (esp_timer_get_time() < then)
    if (xMessageBufferReceive(
          tx_message_buffer.back_handle, &end, sizeof(end), 0u))
      then = esp_timer_get_time() + 500u;

  return end - begin;
}

/// \todo document
/// Wait either 100ms after data or 5ms otherwise
uint8_t receive_acks(uint32_t us, Packet const& packet) {
  if (auto const pulse_width{measure_pulse_width(us)}) {
    static constexpr auto single_pulse_width{156u};
    auto const diff_to_single{
      std::abs(static_cast<int32_t>(pulse_width - single_pulse_width))};
    auto const diff_to_double{
      std::abs(static_cast<int32_t>(pulse_width - 2 * single_pulse_width))};
    printf(
      "width %u    diffs %d/%d\n", pulse_width, diff_to_single, diff_to_double);
    if (size(packet) > 1uz && diff_to_single < diff_to_double)
      gpio_set_level(GPIO_NUM_2, gpio2_state = !gpio2_state);
    return pulse_width < 300uz ? 1u : 2u;
    // return diff_to_single < diff_to_double ? 1u : 2u;
  }
  //
  else
    return 0u;
}

/// \todo document
esp_err_t transmit_acks(uint8_t acks) {
  return xMessageBufferSend(
           out::rx_message_buffer.handle, &acks, sizeof(acks), 0u) ==
             sizeof(acks)
           ? ESP_OK
           : ESP_FAIL;
}

/// \todo document
esp_err_t loop() {
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_1300mA));

  for (;;) {
    // Return on empty packet, suspend or short circuit
    if (auto const packet{receive_packet()};
        !packet || std::to_underlying(state.load() &
                                      (State::Suspend | State::ShortCircuit)))
      return rmt_tx_wait_all_done(channel, -1);
    // Transmit packet
    else {
      ESP_ERROR_CHECK(transmit_packet_blocking(*packet));
      auto const acks{
        receive_acks(size(*packet) > 1uz ? 100'000u : 5000u, *packet)};
      ESP_ERROR_CHECK(transmit_acks(acks));
    }
  }
}

/// \todo document that this pings a decoder (default MX645)
esp_err_t test_loop(uint8_t decoder_id = 221u) {
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_1300mA));

  for (auto i{0uz}; i < 200uz; ++i) {
    Packet packet{0xEFu};
    ESP_ERROR_CHECK(transmit_packet_blocking(packet));
    receive_acks(300u, packet);
    packet[0uz] = 0xBFu;
    ESP_ERROR_CHECK(transmit_packet_blocking(packet));
    receive_acks(300u, packet);
  }

  Packet packet{decoder_id};
  ESP_ERROR_CHECK(transmit_packet_blocking(packet));
  auto const acks{receive_acks(size(packet) > 1uz ? 100'000u : 5000u, packet)};
  if (acks == 2uz) {
    LOGI("DECUP test success");
    return ESP_OK;
  } else {
    LOGE("DECUP test failure");
    return ESP_FAIL;
  }
}

} // namespace

/// \todo document
void task_function(void*) {
  for (;;) switch (decup_encoder_config_t encoder_config{}; state.load()) {
      case State::DECUPZsu: [[fallthrough]];
      case State::DECUP_EIN:
        ESP_ERROR_CHECK(resume(encoder_config, ack_isr_handler));
        ESP_ERROR_CHECK(loop());
        ESP_ERROR_CHECK(suspend());
        break;
      default: LOGI_TASK_SUSPEND(task.handle); break;
    }
}

} // namespace out::track::decup