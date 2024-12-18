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
bool gpio21_state{};

/// \todo document
void IRAM_ATTR ack_isr_handler(void*) {
  uint64_t value{};
  gptimer_get_raw_count(gptimer, &value);

  // Send a notification to the task
  xTaskNotifyIndexedFromISR(task.handle,
                            default_notify_index,
                            static_cast<uint32_t>(value),
                            eSetValueWithOverwrite,
                            NULL);

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

  // Clear any stored counts
  xTaskNotifyStateClearIndexed(NULL, default_notify_index);

  // Wait
  return rmt_tx_wait_all_done(channel, -1);
}

/// \todo document
/// Wait either 100ms after data or 5ms otherwise
uint8_t receive_acks(uint32_t us, bool use_printf = false) {
  uint8_t retval{};

  // Get reference value
  uint32_t begin{};
  auto then{esp_timer_get_time() + us};
  while (esp_timer_get_time() < then) {
    // gpio_set_level(GPIO_NUM_1, gpio1_state = !gpio1_state);
    begin = ulTaskNotifyTakeIndexed(default_notify_index, pdTRUE, 0u);
    if (begin) break;
  }

  // Got nothing
  if (!begin) return retval;

  // As long as end keeps changing every 500us loop
  uint32_t end{begin};
  then = esp_timer_get_time() + 500u;
  while (esp_timer_get_time() < then) {
    gpio_set_level(GPIO_NUM_21, gpio21_state = !gpio21_state);
    auto const tmp{ulTaskNotifyTakeIndexed(default_notify_index, pdTRUE, 0u)};
    if (!tmp || end == tmp) continue;
    end = tmp;
    then = esp_timer_get_time() + 500u;
  }

  if (use_printf) printf("begin %u    end %u\n", begin, end);

  return retval;
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
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_500mA));

  for (;;) {
    // Return on empty packet, suspend or short circuit
    if (auto const packet{receive_packet()};
        !packet || std::to_underlying(state.load() &
                                      (State::Suspend | State::ShortCircuit)))
      return rmt_tx_wait_all_done(channel, -1);
    // Transmit packet
    else {
      ESP_ERROR_CHECK(transmit_packet_blocking(*packet));
      auto const acks{receive_acks((size(*packet) > 1uz ? 100'000u : 5000u))};
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
    receive_acks(300u);
    packet[0uz] = 0xBFu;
    ESP_ERROR_CHECK(transmit_packet_blocking(packet));
    receive_acks(300u);
  }

  Packet packet{decoder_id};
  ESP_ERROR_CHECK(transmit_packet_blocking(packet));
  gpio_set_level(GPIO_NUM_2, 1u);
  auto const acks{receive_acks((size(packet) > 1uz ? 100'000u : 5000u), true)};
  gpio_set_level(GPIO_NUM_2, 0u);

  /// \todo remove
  printf("done\n");
  for (;;) vTaskDelay(pdMS_TO_TICKS(5000u));

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
        ESP_ERROR_CHECK(test_loop());
        ESP_ERROR_CHECK(suspend());
        break;
      default: LOGI_TASK_SUSPEND(task.handle); break;
    }
}

} // namespace out::track::decup