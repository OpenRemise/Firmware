/// DCC task function
///
/// \file   out/track/dcc/task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "task_function.hpp"
#include <driver/gpio.h>
#include <dcc/dcc.hpp>
#include <ztl/fail.hpp>
#include <ztl/inplace_deque.hpp>
#include "log.h"
#include "resume.hpp"

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
      BiDiTCSMin + offsets.tcs)};
  gptimer_set_alarm_action(gptimer, &alarm_config);

  // TODO REMOVE
  gpio_set_level(d20_gpio_num, d20_state = !d20_state);

  return pdFALSE;
}

/// TODO
bool IRAM_ATTR gptimer_callback(gptimer_handle_t timer,
                                gptimer_alarm_event_data_t const* edata,
                                void*) {
  // TCS
  if (edata->alarm_value < BiDiTTS1) {
    gptimer_set_raw_count(timer, BiDiTCSMin);

    // Pull tracks low
    // TODO
    gpio_set_level(d21_gpio_num, true);

    // Reset alarm to TS2
    gptimer_alarm_config_t const alarm_config{.alarm_count = BiDiTTC1};
    gptimer_set_alarm_action(timer, &alarm_config);
  }
  // TS2
  else if (edata->alarm_value < BiDiTTS2) {
    // Reset alarm to TCE
    gptimer_alarm_config_t const alarm_config{.alarm_count = BiDiTCEMin};
    gptimer_set_alarm_action(timer, &alarm_config);
  }
  // TCE
  else {
    // Release tracks
    // TODO
    gpio_set_level(d21_gpio_num, false);

    gptimer_set_alarm_action(gptimer, NULL);
  }

  return pdFALSE;
}

/// TODO
esp_err_t transmit_packet(Packet const& packet) {
  static constexpr rmt_transmit_config_t config{.flags = {.eot_level = 1u}};
  return rmt_transmit(channel, encoder, data(packet), size(packet), &config);
}

/// TODO
void operations_loop() {
  static constexpr auto idle_packet{make_idle_packet()};
  static constexpr auto reset_packet{make_reset_packet()};
  ztl::inplace_deque<Address, trans_queue_depth> addrs{};

  // Preload idle packets
  for (auto i{0uz}; i < trans_queue_depth; ++i) {
    auto const packet{idle_packet};
    ESP_ERROR_CHECK(transmit_packet(packet));
    addrs.push_back(decode_address(data(packet)));
  }

  for (;;) {
    Packet packet{0b00001111u, 0b00111100u, 0b11110000u};

    ESP_ERROR_CHECK(transmit_packet(packet));
    // ESP_ERROR_CHECK(gpio_set_level(d21_gpio_num, d21_state = !d21_state));

    // RailCom stuff
    // Read notification value and so on?
    // Pop queue?

    // Push addr from current packet to queue
  }
}

}  // namespace

/// TODO
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    static_assert(OPTIMIZATION == std::string_view{"-Og"});

    dcc_encoder_config_t encoder_config{
      .num_preamble = 17u,
      .cutoutbit_duration = 60u,
      .bit1_duration = 58u,
      .bit0_duration = 100u,
      .endbit_duration = static_cast<uint8_t>(58u - offsets.endbit),
      .flags{
        .zimo0 = true,
      }};

    ESP_ERROR_CHECK(resume(encoder_config, rmt_callback, gptimer_callback));

    operations_loop();
  }
}

}  // namespace out::track::dcc