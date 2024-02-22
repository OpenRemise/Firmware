/// DCC task function
///
/// \file   out/track/dcc/task_function.cpp
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
#include "analog/convert.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "resume.hpp"
#include "suspend.hpp"

namespace out::track::dcc {

using namespace ::dcc;
using namespace ::dcc::bidi;

namespace {

// TODO REMOVE
bool d20_state{};
bool d21_state{};

/// TODO
struct Offsets {
  uint8_t endbit{};
  uint8_t tcs{};
};

/// TODO
consteval Offsets make_offsets() {
  // TODO REMOVE once we measured all optimization offsets
  static_assert(OPTIMIZATION == std::string_view{"-Og"});

  using namespace std::literals;
  if constexpr (OPTIMIZATION == "-Og"sv)
    return {
      .endbit = 30u,
      .tcs = 21u,
    };
  else if constexpr (OPTIMIZATION == "-Os"sv)
    return {
      .endbit = 0u,
      .tcs = 0u,
    };
  else if constexpr (OPTIMIZATION == "-O2"sv)
    return {
      .endbit = 0u,
      .tcs = 0u,
    };
  else if constexpr (OPTIMIZATION == "-O0"sv)
    return {
      .endbit = 0u,
      .tcs = 0u,
    };
  else { ztl::fail(); }
}

/// TODO
auto const offsets{make_offsets()};

/// TODO
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

/// TODO
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

    // TODO REMOVE DEBUG ONLY
    // gpio_set_level(d21_gpio_num, 1u);
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

    // TODO REMOVE DEBUG ONLY
    // gpio_set_level(d21_gpio_num, 0u);
  }

  return high_task_awoken == pdTRUE;
}

/// TODO
dcc_encoder_config_t dcc_encoder_config() {
  mem::nvs::Settings nvs;
  // return {
  //   .num_preamble = nvs.getDccPreamble(),
  //   .bit1_duration = nvs.getDcc1Duration(),
  //   .bit0_duration = nvs.getDcc0Duration(),
  //   .bidi = nvs.getDccBiDi(),
  // };
  return {.num_preamble = 21u,
          .bidibit_duration = 60u,
          // .bidibit_duration = 0u,
          .bit1_duration = 58u,
          .bit0_duration = 100u,
          .endbit_duration = static_cast<uint8_t>(58u - offsets.endbit)};
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

/// TODO
esp_err_t transmit_packet(Packet const& packet) {
  static constexpr rmt_transmit_config_t config{.flags = {.eot_level = 1u}};
  return rmt_transmit(channel, encoder, data(packet), size(packet), &config);
}

/// TODO
Datagram<> receive_bidi(Address addr) {
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

/// TODO
esp_err_t transmit_bidi(Address addr, Datagram<> const& datagram) {
  RxQueue::value_type const addr_datagram{.addr = addr, .datagram = datagram};
  return xQueueSend(rx_queue.handle, &addr_datagram, 0u) ? ESP_OK : ESP_FAIL;
}

/// TODO
esp_err_t operations_loop() {
  static constexpr auto idle_packet{make_idle_packet()};
  ztl::inplace_deque<Packet, trans_queue_depth> packets{};
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};

  // Preload idle packets
  for (auto i{0uz}; i < trans_queue_depth; ++i) {
    packets.push_back(idle_packet);
    ESP_ERROR_CHECK(transmit_packet(packets.front()));
  }

  for (;;) {
    // Receive BiDi on last transmitted address
    auto const addr{decode_address(data(*(begin(packets) - 1)))};
    auto const datagram{receive_bidi(addr)};
    transmit_bidi(addr, datagram);
    packets.pop_front();

    // Return on timeout
    if (auto const now{xTaskGetTickCount()}; now >= then)
      return rmt_tx_wait_all_done(channel, -1);
    // In case we got data, reset timeout
    else if (auto const packet{receive_packet()}) {
      then = now + pdMS_TO_TICKS(task.timeout);
      packets.push_back(*packet);
    }
    // We got no data, transmit idle packet
    else
      packets.push_back(idle_packet);

    // Transmit packet
    ESP_ERROR_CHECK(transmit_packet(packets.front()));
  }
}

/// TODO
analog::CurrentsQueue::value_type peek_current_measurements() {
  analog::CurrentsQueue::value_type currents;
  assert(xQueuePeek(analog::currents_queue.handle, &currents, 0u));
  return currents;
}

/// TODO
analog::CurrentMeasurement mean_current_measurement() {
  auto const currents{peek_current_measurements()};
  return static_cast<analog::CurrentMeasurement>(
    std::accumulate(cbegin(currents), cend(currents), 0) / size(currents));
}

/// TODO
template<std::ranges::contiguous_range R>
void append_current_measurements(R&& r) {
  auto const currents{peek_current_measurements()};
  if (size(r) < size(currents) ||
      !std::equal(cbegin(currents), cend(currents), cend(r) - size(currents)))
    std::ranges::copy(currents, std::back_inserter(r));
}

/// TODO
template<std::ranges::contiguous_range R>
bool detect_ack(R&& r, analog::CurrentMeasurement ref) {
  // ACKs must be at least 5ms long
  static constexpr auto wlen{
    static_cast<int>(5e-3 * (analog::sample_freq_hz / size(analog::channels)))};

  // TODO this should be a setting
  auto const delta{mA2measurement(analog::Current{60})};

  //
  for (auto const windows{r | std::views::slide(wlen)};
       auto const& window : windows) {
    auto const movsum{(std::accumulate(cbegin(window), cend(window), 0))};
    if (movsum - wlen * ref > wlen * delta) return true;
  }
  return false;
}

/// TODO
esp_err_t transmit_ack(bool ack) {
  return xMessageBufferSend(rx_message_buffer.handle, &ack, sizeof(ack), 0u) ==
             sizeof(ack)
           ? ESP_OK
           : ESP_FAIL;
}

/// TODO
esp_err_t service_loop() {
  static constexpr auto reset_packet{make_reset_packet()};
  static constexpr auto read_timeout{pdMS_TO_TICKS(50u)};
  static constexpr auto write_timeout{pdMS_TO_TICKS(100u)};
  ztl::inplace_deque<Packet, trans_queue_depth> packets{reset_packet};
  analog::CurrentMeasurement ref_current{};
  ztl::inplace_vector<analog::CurrentMeasurement::value_type, 1024uz>
    current_measurements{};

  // Transmit 25 reset packets to ensure entry
  for (auto i{0uz}; i < 25uz + 3uz; ++i)
    ESP_ERROR_CHECK(transmit_packet(packets.front()));

  for (;;) {
    // Transmit 3 reset packets to ensure sequence
    for (auto i{0uz}; i < 3uz; ++i) {
      packets.push_back(reset_packet);
      ESP_ERROR_CHECK(transmit_packet(packets.front()));
      packets.pop_front();
    }

    // Transmit reset packets until first non-reset packet or timeout
    TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};
    do {
      // Return on timeout
      if (auto const now{xTaskGetTickCount()}; now >= then)
        return rmt_tx_wait_all_done(channel, -1);
      // In case we got data, reset timeout
      else if (auto const packet{receive_packet()}) {
        then = now + pdMS_TO_TICKS(task.timeout);
        packets.push_back(*packet);
      }
      // We got no data, transmit idle packet
      else
        packets.push_back(reset_packet);

      ESP_ERROR_CHECK(transmit_packet(packets.front()));
      packets.pop_front();

      // Take reference current
      ref_current = mean_current_measurement();
    } while (packets.back() == reset_packet);

    // Transmit equal CV access packets, try to detect ack
    // TODO read timeout would theoretically be only 50ms?
    auto const cv_access_packet{packets.back()};
    then = xTaskGetTickCount() + write_timeout +
           pdMS_TO_TICKS(trans_queue_depth * 10u);
    bool ack{};
    do {
      if (auto const packet{receive_packet()}) packets.push_back(*packet);
      else break;
      ESP_ERROR_CHECK(transmit_packet(packets.front()));
      packets.pop_front();
      append_current_measurements(current_measurements);
      ack |= detect_ack(current_measurements, ref_current);
    } while (packets.back() == cv_access_packet);

    // Transmit reset packets until ack or timeout
    while (!ack && xTaskGetTickCount() < then) {
      packets.push_back(reset_packet);
      ESP_ERROR_CHECK(transmit_packet(packets.front()));
      packets.pop_front();
      append_current_measurements(current_measurements);
      ack |= detect_ack(current_measurements, ref_current);
    }
    ESP_ERROR_CHECK(transmit_ack(ack));
    current_measurements.clear();
  }
}

}  // namespace

/// TODO
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    switch (auto encoder_config{dcc_encoder_config()}; mode.load()) {
      case Mode::DCC_EIN: [[fallthrough]];
      case Mode::DCCOperations:
        ESP_ERROR_CHECK(resume(encoder_config, rmt_callback, gptimer_callback));
        ESP_ERROR_CHECK(operations_loop());
        break;
      case Mode::DCCService:
        // RCN-216 demands at least 20 preamble bits
        encoder_config.num_preamble =
          std::max<decltype(encoder_config.num_preamble)>(
            encoder_config.num_preamble, 20u);
        encoder_config.bidibit_duration = 0u;
        ESP_ERROR_CHECK(resume(encoder_config, nullptr, nullptr));
        ESP_ERROR_CHECK(service_loop());
        break;
      default: assert(false); break;
    }

    ESP_ERROR_CHECK(suspend());
  }
}

}  // namespace out::track::dcc