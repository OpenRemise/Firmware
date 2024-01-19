/// DCC task function
///
/// \file   out/track/dcc/task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "task_function.hpp"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <hal/uart_hal.h>
#include <rmt_dcc_encoder.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <dcc/dcc.hpp>
#include <limits>
#include "../utility.hpp"
#include "analog/convert.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "resume.hpp"
#include "suspend.hpp"

namespace out::track::dcc {

using namespace ::dcc;
using namespace ::dcc::bidi;

namespace {

/// TODO
bool IRAM_ATTR bidi_rmt_callback(rmt_channel_handle_t,
                                 rmt_tx_done_event_data_t const*,
                                 void*) {
  gptimer_set_raw_count(gptimer, 0u);
  gptimer_alarm_config_t const alarm_config{
    .alarm_count = ::dcc::Timing::BiDiTTC1 - ::dcc::Timing::BiDiTCSMin};
  gptimer_set_alarm_action(gptimer, &alarm_config);

  // Pull tracks low
  gpio_set_level(bidi_en_gpio_num, true);

  return pdFALSE;
}

/// TODO
bool IRAM_ATTR bidi_gptimer_callback(gptimer_handle_t timer,
                                     gptimer_alarm_event_data_t const* edata,
                                     void*) {
  static bool ch1{};
  bool const ch2{true};
  BaseType_t high_task_awoken{pdFALSE};

  // End of channel 1
  if (edata->alarm_value ==
      ::dcc::Timing::BiDiTTC1 - ::dcc::Timing::BiDiTCSMin) {
    // Reset alarm to end of channel 2
    gptimer_alarm_config_t const alarm_config{
      .alarm_count = edata->alarm_value +
                     (::dcc::Timing::BiDiTTC2 - ::dcc::Timing::BiDiTTC1)};
    gptimer_set_alarm_action(timer, &alarm_config);

    // Check whether there has been data in channel 1
    ch1 = uart_ll_get_rxfifo_len(&UART1) >= 2uz;
  }
  // End of channel 2
  else {
    // Release tracks
    gpio_set_level(bidi_en_gpio_num, false);

    // Send a notification to unblock the task
    xTaskNotifyIndexedFromISR(task.handle,
                              default_notify_index,
                              (ch2 << 1u) | ch1,
                              eSetValueWithOverwrite,
                              &high_task_awoken);
  }

  return high_task_awoken == pdTRUE;
}

/// TODO
bool IRAM_ATTR rmt_callback(rmt_channel_handle_t,
                            rmt_tx_done_event_data_t const*,
                            void*) {
  BaseType_t high_task_awoken{pdFALSE};
  vTaskNotifyGiveIndexedFromISR(
    task.handle, default_notify_index, &high_task_awoken);
  return high_task_awoken == pdTRUE;
}

/// TODO
dcc_encoder_config_t dcc_encoder_config() {
  mem::nvs::Settings nvs;
  return {
    .num_preamble = nvs.getDccPreamble(),
    .bit1_duration = nvs.getDcc1Duration(),
    .bit0_duration = nvs.getDcc0Duration(),
    .bidi = nvs.getDccBiDi(),
  };
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
esp_err_t transmit_packet(Packet const& packet, auto const& tx_configs) {
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

/// TODO
/// don't forget that this is BiDi data from the previous packet!!!
Datagram<> receive_bidi(uint32_t last_notification_value) {
  Datagram datagram{};
  auto const bytes_available{uart_ll_get_rxfifo_len(&UART1)};

  // CH1+2
  if (last_notification_value & 0b1u)
    uart_ll_read_rxfifo(&UART1,
                        data(datagram),
                        std::min<size_t>(bytes_available, size(datagram)));
  // CH2 only
  else
    uart_ll_read_rxfifo(
      &UART1,
      data(datagram) + channel1_size,
      std::min<size_t>(bytes_available, size(datagram) - channel1_size));

  uart_ll_rxfifo_rst(&UART1);

  return datagram;
}

/// TODO
void transmit_bidi(Address last_addr, Datagram<> const& datagram) {
  RxQueue::value_type addr_datagram{.addr = last_addr, .datagram = datagram};
  if (!xQueueSend(rx_queue.handle, &addr_datagram, 0u))
    ;  // I'd love to error log here, but it's simply too slow
}

/// TODO
void operations_loop(std::array<rmt_transmit_config_t, 2uz> const& tx_configs) {
  static constexpr auto idle_packet{make_idle_packet()};
  Address last_addr{};
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};
  uint32_t last_notification_value{};

  for (;;) {
    auto packet{receive_packet()};

    // Return on timeout
    if (auto const now{xTaskGetTickCount()}; now >= then) return;
    // In case we got data, reset timeout
    else if (packet) then = now + pdMS_TO_TICKS(task.timeout);
    else packet = idle_packet;

    ESP_ERROR_CHECK(transmit_packet(*packet, tx_configs));

    auto const datagram{receive_bidi(last_notification_value)};
    transmit_bidi(last_addr, datagram);

    last_addr = ::dcc::decode_address(data(*packet));

    last_notification_value = notify_take_sync_reset(portMAX_DELAY);

    // No more work here! This is timing critical!
  }
}

/// TODO
bool detect_ack() {
  analog::CurrentsQueue::value_type currents;
  xQueuePeek(analog::currents_queue.handle, &currents, 0u);
  std::ranges::sort(currents);
  // ACK is at least 5ms long, n is the number of samples we take in that time
  constexpr auto n{static_cast<int32_t>(
    5e-3 * (analog::sample_freq_hz / size(analog::channels)))};
  static_assert(n == 2);
  constexpr auto delta{60};
  auto const n_max{std::accumulate(cend(currents) - n, cend(currents), 0)};
  auto const n_min{std::accumulate(cbegin(currents), cbegin(currents) + n, 0)};
  return measurement2mA(
           static_cast<analog::CurrentMeasurement>(n_max - n_min)) >= n * delta;
}

/// TODO
void transmit_ack(bool ack) {
  if (!xMessageBufferSend(rx_message_buffer.handle, &ack, sizeof(ack), 0u))
    ;
}

/// TODO
void service_loop(std::array<rmt_transmit_config_t, 2uz> const& tx_configs) {
  static constexpr auto reset_packet{make_reset_packet()};
  std::optional<Packet> packet{reset_packet};

  // Transmit 25 reset packets to ensure entry
  for (auto i{0uz}; i < 25uz; ++i) {
    ESP_ERROR_CHECK(transmit_packet(*packet, tx_configs));
    notify_take_sync_reset(portMAX_DELAY);
  }

  for (;;) {
    // Transmit reset packets until first non-reset packet or timeout
    TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};
    do {
      packet = receive_packet();

      // Return on timeout
      if (auto const now{xTaskGetTickCount()}; now >= then) return;
      // In case we got data, reset timeout
      else if (packet) then = now + pdMS_TO_TICKS(task.timeout);
      else packet = reset_packet;

      ESP_ERROR_CHECK(transmit_packet(*packet, tx_configs));
      notify_take_sync_reset(portMAX_DELAY);
    } while (packet == reset_packet);

    // Transmit equal CV access packets, try to detect ack
    // TODO read timeout would theoretically be only 50ms?
    then = xTaskGetTickCount() + pdMS_TO_TICKS(100u);
    auto const cv_access_packet{*packet};
    bool ack{};
    do {
      packet = receive_packet();
      if (!packet) break;
      ESP_ERROR_CHECK(transmit_packet(*packet, tx_configs));
      ack |= detect_ack();
      notify_take_sync_reset(portMAX_DELAY);
    } while (packet == cv_access_packet);

    // Transmit reset packets until timeout or ack detected
    packet = reset_packet;
    while (xTaskGetTickCount() < then && !ack) {
      ESP_ERROR_CHECK(transmit_packet(*packet, tx_configs));
      ack |= detect_ack();
      notify_take_sync_reset(portMAX_DELAY);
    }
    transmit_ack(ack);

    // Transmit reset packets until current drops (safety measure so that ack
    // isn't counted twice...)
    while (ack && detect_ack()) {
      ESP_ERROR_CHECK(transmit_packet(*packet, tx_configs));
      notify_take_sync_reset(portMAX_DELAY);
    }
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
        ESP_ERROR_CHECK(
          resume(encoder_config,
                 encoder_config.bidi ? bidi_rmt_callback : rmt_callback,
                 encoder_config.bidi ? bidi_gptimer_callback : nullptr));
        // Invert second EOT level for BiDi (since the channel itself is
        // inverted this means it will pull to ground after the packet end bit)
        operations_loop(
          {rmt_transmit_config_t{},
           rmt_transmit_config_t{.flags = {.eot_level = encoder_config.bidi}}});
        break;
      case Mode::DCCService:
        // RCN-216 demands at least 20 preamble bits
        encoder_config.num_preamble =
          std::max<decltype(encoder_config.num_preamble)>(
            encoder_config.num_preamble, 20u);
        encoder_config.bidi = false;
        ESP_ERROR_CHECK(resume(encoder_config, rmt_callback, nullptr));
        service_loop({});
        break;
      default: assert(false); break;
    }
    ESP_ERROR_CHECK(suspend());
  }
}

}  // namespace out::track::dcc