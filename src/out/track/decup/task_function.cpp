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

namespace out::track::decup {

using namespace ::decup;
using ::ulf::decup_ein::ack, ::ulf::decup_ein::nak;

namespace {

bool d20_state{};
bool d21_state{};

/// \todo document
void IRAM_ATTR ack_isr_handler(void*) {
  xTaskNotifyIndexedFromISR(
    task.handle, default_notify_index, 0u, eIncrement, NULL);

  gpio_set_level(d20_gpio_num, d20_state = !d20_state);
}

/// \todo document
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

/// \todo document
/// \TODO EOT level for last transmission MUST be 0!!! Otherwise DRV8328 INHA
/// stays high
esp_err_t transmit_packet_blocking(Packet const& packet) {
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

/// \todo document
uint8_t receive_acks(uint32_t us) {
  uint8_t retval{};

  // Wait either
  // 100ms after data or
  // 5ms otherwise
  auto const then{esp_timer_get_time() + us};
  while (esp_timer_get_time() < then)
    if (retval += static_cast<uint8_t>(
          ulTaskNotifyTakeIndexed(default_notify_index, pdTRUE, 0u));
        retval == 2u)
      break;
  return retval;
}

/// \todo document
esp_err_t transmit_acks(uint8_t acks) {
  return xMessageBufferSend(out::rx_message_buffer.handle,
                            &acks,
                            sizeof(acks),
                            pdMS_TO_TICKS(task.timeout)) == sizeof(acks)
           ? ESP_OK
           : ESP_FAIL;
}

/// Just run preamble and then pulse a MX645...
/// This should produce a double pulse response
esp_err_t test_entry() {
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_500mA));
  for (auto i{0uz}; i < 200uz; ++i) {
    Packet packet{0xEFu};
    ESP_ERROR_CHECK(transmit_packet_blocking(packet));
    receive_acks(300u);
    packet[0uz] = 0xBFu;
    ESP_ERROR_CHECK(transmit_packet_blocking(packet));
    receive_acks(300u);
  }

  // 221 -> MX645
  gpio_set_level(d21_gpio_num, d21_state = !d21_state);
  Packet packet{221};
  ESP_ERROR_CHECK(transmit_packet_blocking(packet));
  auto const acks{receive_acks((size(packet) > 1uz ? 100'000u : 5000u))};
  LOGI("MX654 DECUP number of ack pulses %d\n", acks);

  for (;;) { vTaskDelay(pdMS_TO_TICKS(200u)); }

  return ESP_OK;
}

/// \todo document
esp_err_t loop() {
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_500mA));
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};

  for (;;) {
    auto const packet{receive_packet()};

    // Return on timeout, suspend or short circuit
    if (auto const now{xTaskGetTickCount()};
        now >= then || std::to_underlying(
                         state.load() & (State::Suspend | State::ShortCircuit)))
      return rmt_tx_wait_all_done(channel, -1);
    // In case we got a packet, reset timeout
    else if (packet) {
      then = now + pdMS_TO_TICKS(task.timeout);
      ESP_ERROR_CHECK(transmit_packet_blocking(*packet));
      auto const acks{receive_acks((size(*packet) > 1uz ? 100'000u : 5000u))};
      ESP_ERROR_CHECK(transmit_acks(acks));
    }
  }
}

}  // namespace

/// \todo document
void task_function(void*) {
  for (;;) switch (decup_encoder_config_t encoder_config{}; state.load()) {
      case State::DECUP_EIN:
        ESP_ERROR_CHECK(resume(encoder_config, ack_isr_handler));
        ESP_ERROR_CHECK(loop());
        ESP_ERROR_CHECK(suspend());
        break;
      default: LOGI_TASK_SUSPEND(task.handle); break;
    }
}

}  // namespace out::track::decup