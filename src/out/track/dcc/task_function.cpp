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
#include <ztl/fail.hpp>
#include <ztl/inplace_deque.hpp>
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "resume.hpp"
#include "suspend.hpp"

namespace out::track::dcc {

using namespace ::dcc;
using namespace ::dcc::bidi;

namespace {

bool d20_state{};  // ch2
bool d21_state{};  // ch3

struct Offsets {
  uint8_t endbit{};
  uint8_t tcs{};
};

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

auto const offsets{make_offsets()};

/// TODO
bool IRAM_ATTR rmt_callback(rmt_channel_handle_t,
                            rmt_tx_done_event_data_t const*,
                            void*) {
  gptimer_set_raw_count(gptimer, 0u);
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
    gpio_set_level(bidi_en_gpio_num, true);

    // Reset alarm to TS2
    gptimer_alarm_config_t const alarm_config{.alarm_count = TTC1};
    gptimer_set_alarm_action(timer, &alarm_config);
  }
  // TS2
  else if (edata->alarm_value < TTS2) {
    // Check whether there has been data in channel 1
    ch1 = uart_ll_get_rxfifo_len(&UART1) >= 2uz;

    // Reset alarm to TCE
    gptimer_alarm_config_t const alarm_config{.alarm_count = TCEMin};
    gptimer_set_alarm_action(timer, &alarm_config);

    // TODO REMOVE DEBUG ONLY
    gpio_set_level(d21_gpio_num, true);
  }
  // TCE
  else {
    // Release tracks
    gpio_set_level(bidi_en_gpio_num, false);

    // Send a notification to the task
    xTaskNotifyIndexedFromISR(task.handle,
                              default_notify_index,
                              (ch2 << 1u) | ch1,
                              eSetValueWithOverwrite,
                              &high_task_awoken);

    gptimer_set_alarm_action(gptimer, NULL);

    // TODO REMOVE DEBUG ONLY
    gpio_set_level(d21_gpio_num, false);
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
          .cutoutbit_duration = 60u,
          // .cutoutbit_duration = 0u,
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
esp_err_t receive_bidi(Address addr) {
  //
  auto const notification_value{
    ulTaskNotifyTakeIndexed(pdTRUE, default_notify_index, 0u)};

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

  uart_ll_rxfifo_rst(&UART1);

  RxQueue::value_type addr_datagram{.addr = addr, .datagram = datagram};
  if (!xQueueSend(rx_queue.handle, &addr_datagram, 0u))
    ;  // I'd love to error log here, but it's simply too slow

  return ESP_OK;
}

// #define FORCE_ERROR

/// TODO
void operations_loop() {
  static constexpr auto idle_packet{make_idle_packet()};
  ztl::inplace_deque<Packet, trans_queue_depth + 1uz> packets{};
  ztl::inplace_deque<Address, trans_queue_depth + 1uz> addrs{};
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(task.timeout)};

  // Preload idle packets
  for (auto i{0uz}; i < trans_queue_depth; ++i) {
    packets.push_back(idle_packet);
    addrs.push_back(decode_address(data(idle_packet)));
    ESP_ERROR_CHECK(transmit_packet(packets.back()));
  }

  Packet old_packet{};
  bool toggle{};
  for (;;) {
    // auto packet{receive_packet()};

    // // Return on timeout
    // if (auto const now{xTaskGetTickCount()}; now >= then) return;
    // // In case we got data, reset timeout
    // else if (packet) then = now + pdMS_TO_TICKS(task.timeout);
    // else packet = idle_packet;

    if (toggle) {
      // 21*2*58        Preamble
      // 4*2*100        Startbits
      // 4*8*2*58       Bytes
      // 2*58           Endbit
      old_packet = {0x03u, 0xDFu, 0x00, 0xDCu};
      packets.push_back(old_packet);
      assert(size(old_packet) == 4uz);
    } else {
      old_packet = {0x03u, 0xA0u, 0xA3u};
      packets.push_back(old_packet);
      assert(size(old_packet) == 3uz);
    }
    toggle = !toggle;

    //
    addrs.push_back(decode_address(data(packets.back())));

    //
#ifdef FORCE_ERROR
    ESP_ERROR_CHECK(transmit_packet(old_packet));
#else
    ESP_ERROR_CHECK(transmit_packet(packets.front()));
#endif

    //
    ESP_ERROR_CHECK(receive_bidi(addrs.front()));

    // Done with those
    packets.pop_front();
    addrs.pop_front();
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
        operations_loop();
        break;
      case Mode::DCCService: assert(false); break;
      default: assert(false); break;
    }

    ESP_ERROR_CHECK(suspend());
  }
}

}  // namespace out::track::dcc