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

/// DCC task function
///
/// \file   drv/out/track/dcc/task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "task_function.hpp"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <hal/uart_hal.h>
#include <dcc/dcc.hpp>
#include <ranges>
#include <ztl/fail.hpp>
#include <ztl/inplace_deque.hpp>
#include "../current_limit.hpp"
#include "drv/analog/convert.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "resume.hpp"
#include "suspend.hpp"

namespace drv::out::track::dcc {

using namespace ::dcc;
using namespace ::dcc::bidi;
using namespace std::literals;

namespace {

/// \todo remove
bool gpio1_state{};
bool gpio2_state{};

/// \todo document
struct Offsets {
  uint8_t endbit{};
  uint8_t tcs{};
};

/// \todo document
/// \bug DCC/RailCom timings seem to get worse when the DCC tasks runs the
/// second time?
consteval Offsets make_offsets() {
  static_assert(CONFIG_IDF_INIT_VERSION == "5.5.0"sv);

  return {
#if defined(CONFIG_COMPILER_OPTIMIZATION_DEBUG)
    .endbit = 34u,
    .tcs = 21u,
#elif defined(CONFIG_COMPILER_OPTIMIZATION_SIZE)
    .endbit = 31u,
    .tcs = 20u,
#else
#  error "Unknown optimization setting"
#endif
  };
}

/// \todo document
auto const offsets{make_offsets()};

/// \todo document
bool IRAM_ATTR rmt_callback(rmt_channel_handle_t,
                            rmt_tx_done_event_data_t const*,
                            void*) {
  gptimer_set_raw_count(gptimer, 0ull);
  gptimer_alarm_config_t const alarm_config{
    .alarm_count = static_cast<decltype(gptimer_alarm_config_t::alarm_count)>(
      TCSMin + offsets.tcs)};
  gptimer_set_alarm_action(gptimer, &alarm_config);
  return pdFALSE;
}

/// \todo document
bool IRAM_ATTR gptimer_callback(gptimer_handle_t timer,
                                gptimer_alarm_event_data_t const* edata,
                                void*) {
  static bool ch1{};
  bool const ch2{true};
  BaseType_t high_task_awoken{pdFALSE};

  // TCS
  if (edata->alarm_value < TTS1) {
    gptimer_set_raw_count(timer, TCSMin);

    // Pull tracks low
    gpio_set_level(bidi_en_gpio_num, 1u);

    // Reset alarm to TS2
    gptimer_alarm_config_t const alarm_config{.alarm_count = TTC1};
    gptimer_set_alarm_action(timer, &alarm_config);
  }
  // TS2
  else if (edata->alarm_value < TTS2) {
    // Reset alarm to TCE
    gptimer_alarm_config_t const alarm_config{.alarm_count = TCEMin};
    gptimer_set_alarm_action(timer, &alarm_config);

    // Check whether there has been data in channel 1
    ch1 = uart_ll_get_rxfifo_len(&UART1);
  }
  // TCE
  else {
    // Release tracks
    gpio_set_level(bidi_en_gpio_num, 0u);

    // Send a notification to the task
    xTaskNotifyIndexedFromISR(task.handle,
                              default_notify_index,
                              (ch2 << 1u) | ch1,
                              eSetValueWithOverwrite,
                              &high_task_awoken);

    gptimer_set_alarm_action(gptimer, NULL);
  }

  return high_task_awoken == pdTRUE;
}

/// \todo document
dcc_encoder_config_t dcc_encoder_config(State dcc_state = state.load()) {
  mem::nvs::Settings nvs;
  auto const bit1_duration{nvs.getDccBit1Duration()};
  dcc_encoder_config_t retval{
    .num_preamble = nvs.getDccPreamble(),
    .bidibit_duration = nvs.getDccBiDiBitDuration(),
    .bit1_duration = bit1_duration,
    .bit0_duration = nvs.getDccBit0Duration(),
    .endbit_duration = static_cast<uint8_t>(bit1_duration - offsets.endbit),
    .flags = {.level0 = true}};

  // Service mode can't do BiDi and RCN-216 demands at least 20 preamble bits
  if (dcc_state == State::DCCService) {
    retval.num_preamble =
      std::max<decltype(retval.num_preamble)>(retval.num_preamble, 20u);
    retval.bidibit_duration = 0u;
  }

  return retval;
}

/// \todo document
std::optional<Packet> receive_packet() {
  Packet packet;
  //
  if (auto const bytes_received{xMessageBufferReceive(
        tx_message_buffer.front_handle, data(packet), packet.max_size(), 0u)}) {
    packet.resize(bytes_received);
    return packet;
  }
  //
  else if (auto const bytes_received{
             xMessageBufferReceive(tx_message_buffer.back_handle,
                                   data(packet),
                                   packet.max_size(),
                                   0u)}) {
    packet.resize(bytes_received);
    return packet;
  }
  //
  else
    return std::nullopt;
}

/// \todo document
esp_err_t transmit_packet(Packet const& packet) {
  static constexpr rmt_transmit_config_t config{};
  return rmt_transmit(channel, encoder, data(packet), size(packet), &config);
}

/// \todo document
Datagram<> receive_bidi() {
  //
  auto const notification_value{
    ulTaskNotifyTakeIndexed(default_notify_index, pdTRUE, portMAX_DELAY)};

  //
  Datagram datagram{};
  auto const bytes_available{uart_ll_get_rxfifo_len(&UART1)};

  // CH1+2
  if (notification_value & 0b1u)
    uart_ll_read_rxfifo(&UART1,
                        data(datagram),
                        std::min<size_t>(bytes_available, size(datagram)));
  // CH2 only
  else
    uart_ll_read_rxfifo(
      &UART1,
      data(datagram) + channel1_size,
      std::min<size_t>(bytes_available, size(datagram) - channel1_size));

  // Flush FIFO
  uart_ll_rxfifo_rst(&UART1);

  return datagram;
}

/// \todo document
esp_err_t transmit_bidi(RxQueue::value_type item) {
  return xQueueSend(rx_queue.handle, &item, 0u) ? ESP_OK : ESP_FAIL;
}

/// \todo document
esp_err_t operations_loop(dcc_encoder_config_t const& encoder_config) {
  static constexpr auto idle_packet{make_idle_packet()};
  ztl::inplace_deque<Packet, trans_queue_depth> packets{};

  // Set current limit from NVS
  ESP_ERROR_CHECK(set_current_limit(mem::nvs::Settings{}.getCurrentLimit()));

  // Preload idle packets
  for (auto i{0uz}; i < trans_queue_depth; ++i) {
    packets.push_back(idle_packet);
    ESP_ERROR_CHECK(transmit_packet(packets.front()));
  }

  for (;;) {
    // Receive BiDi on last transmitted packet
    /// \bug Can't error check here? For some reason transmit_bidi immediately
    /// fails in ULF_DCC_EIN mode.
    if (encoder_config.bidibit_duration)
      transmit_bidi(
        {.packet = *(cbegin(packets) - 1), .datagram = receive_bidi()});
    packets.pop_front();

    // Return on empty packet, suspend or short circuit
    if (auto const packet{receive_packet()};
        !packet || std::to_underlying(state.load() &
                                      (State::Suspend | State::ShortCircuit)))
      return rmt_tx_wait_all_done(channel, -1);
    else packets.push_back(*packet);

    // Transmit packet
    ESP_ERROR_CHECK(transmit_packet(packets.front()));
  }
}

/// \todo document
drv::analog::CurrentsQueue::value_type peek_current_measurements() {
  drv::analog::CurrentsQueue::value_type currents;
  if (!xQueuePeek(drv::analog::currents_queue.handle, &currents, 0u))
    assert(false);
  return currents;
}

/// \todo document
template<std::ranges::contiguous_range R>
void append_current_measurements(R&& r) {
  auto const currents{peek_current_measurements()};
  if (size(r) < size(currents) ||
      !std::equal(cbegin(currents), cend(currents), cend(r) - size(currents)))
    std::ranges::copy(currents, std::back_inserter(r));
}

/// \todo document
/// this is the mean version of ack detection
template<std::ranges::contiguous_range R>
bool detect_ack(R&& r, drv::analog::Current ack_current) {
  // ACKs must be at least 5ms long
  static constexpr auto wlen{static_cast<int>(
    5e-3 * (drv::analog::sample_freq_hz / size(drv::analog::channels)))};
  static_assert(wlen == 20);

  //
  auto const delta_measurement{mA2measurement(ack_current)};

  // Reference value set to mean of first slide
  std::optional<drv::analog::CurrentMeasurement::value_type> ref_measurement;

  //
  for (auto const windows{r | std::views::slide(wlen)};
       auto const& window : windows)
    if (auto const movsum{(std::accumulate(cbegin(window), cend(window), 0))};
        !ref_measurement)
      ref_measurement = movsum;
    else if (static constexpr auto ten_pct{static_cast<size_t>(0.1 * wlen)};
             std::ranges::count(window, 0uz) < ten_pct &&
             movsum - *ref_measurement > wlen * delta_measurement)
      return true;

  return false;
}

/// \todo document
esp_err_t transmit_ack(bool ack) {
  return xMessageBufferSend(rx_message_buffer.handle, &ack, sizeof(ack), 0u) ==
             sizeof(ack)
           ? ESP_OK
           : ESP_FAIL;
}

/// \todo document
/// \todo read timeout would theoretically be only 50ms?
esp_err_t service_loop(dcc_encoder_config_t const&) {
  static constexpr auto reset_packet{make_reset_packet()};
  static constexpr auto read_timeout{50u};
  static constexpr auto write_timeout{100u};
  ztl::inplace_deque<Packet, trans_queue_depth> packets{reset_packet};
  std::vector<drv::analog::CurrentMeasurement::value_type> current_measurements;
  current_measurements.reserve(2048uz);

  mem::nvs::Settings nvs;
  auto const startup_reset_packet_count{nvs.getDccStartupResetPacketCount()};
  auto const continue_reset_packet_count{nvs.getDccContinueResetPacketCount()};
  drv::analog::Current const ack_current{nvs.getDccProgrammingAckCurrent()};
  nvs.~Settings();

  // Transmit at least 25 reset packets to ensure entry
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_4100mA));
  for (auto i{0uz}; i < startup_reset_packet_count + 3uz; ++i)
    ESP_ERROR_CHECK(transmit_packet(packets.front()));

  // Set current limit from NVS
  ESP_ERROR_CHECK(
    set_current_limit(mem::nvs::Settings{}.getCurrentLimitService()));

  for (;;) {
    // Transmit at least 3 reset packets to ensure sequence
    for (auto i{0uz}; i < continue_reset_packet_count; ++i) {
      packets.push_back(reset_packet);
      ESP_ERROR_CHECK(transmit_packet(packets.front()));
      packets.pop_front();
    }

    // Transmit reset packets until first non-reset packet
    do {
      // Return on empty packet, suspend or short circuit
      if (auto const packet{receive_packet()};
          !packet || std::to_underlying(state.load() &
                                        (State::Suspend | State::ShortCircuit)))
        return rmt_tx_wait_all_done(channel, -1);
      else packets.push_back(*packet);
      ESP_ERROR_CHECK(transmit_packet(packets.front()));
      packets.pop_front();
    } while (packets.back() == reset_packet);

    // Transmit equal CV access packets, try to detect ack
    auto const cv_access_packet{packets.back()};
    TickType_t const then{
      xTaskGetTickCount() +
      pdMS_TO_TICKS(write_timeout + trans_queue_depth * 10u)};
    bool ack{};
    do {
      if (auto const packet{receive_packet()}) packets.push_back(*packet);
      else break;
      ESP_ERROR_CHECK(transmit_packet(packets.front()));
      packets.pop_front();
      append_current_measurements(current_measurements);
      ack |= detect_ack(current_measurements, ack_current);
    } while (packets.back() == cv_access_packet);

    // Transmit reset packets until ack or timeout
    while (!ack && xTaskGetTickCount() < then) {
      packets.push_back(reset_packet);
      ESP_ERROR_CHECK(transmit_packet(packets.front()));
      packets.pop_front();
      append_current_measurements(current_measurements);
      ack |= detect_ack(current_measurements, ack_current);
    }
    ESP_ERROR_CHECK(transmit_ack(ack));
    current_measurements.clear();
  }
}

} // namespace

/// \todo document
void task_function(void*) {
  switch (auto encoder_config{dcc_encoder_config()}; state.load()) {
    case State::DCCOperations: [[fallthrough]];
    case State::ULF_DCC_EIN:
      ESP_ERROR_CHECK(
        resume(encoder_config,
               encoder_config.bidibit_duration ? rmt_callback : NULL,
               encoder_config.bidibit_duration ? gptimer_callback : NULL));
      ESP_ERROR_CHECK(operations_loop(encoder_config));
      ESP_ERROR_CHECK(suspend());
      break;
    case State::DCCService:
      ESP_ERROR_CHECK(resume(encoder_config, nullptr, nullptr));
      ESP_ERROR_CHECK(service_loop(encoder_config));
      ESP_ERROR_CHECK(suspend());
      break;
    default: assert(false); break;
  }
  LOGI_TASK_DESTROY();
}

} // namespace drv::out::track::dcc
