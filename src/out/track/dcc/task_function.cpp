/// DCC task function
///
/// \file   out/track/dcc/task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "task_function.hpp"
#include <driver/gpio.h>
#include <dcc/dcc.hpp>
#include <ztl/inplace_deque.hpp>
#include "log.h"
#include "resume.hpp"
#include "rmt_dcc_encoder.h"

namespace out::track::dcc {

using namespace ::dcc;
using namespace ::dcc::bidi;

namespace {

bool d20_state{};  // ch2
bool d21_state{};  // ch3

/// TODO
bool IRAM_ATTR rmt_callback(rmt_channel_handle_t,
                            rmt_tx_done_event_data_t const*,
                            void*) {
  gpio_set_level(d20_gpio_num, d20_state = !d20_state);
  return pdFALSE;
}

/// TODO
bool IRAM_ATTR gptimer_callback(gptimer_handle_t timer,
                                gptimer_alarm_event_data_t const* edata,
                                void*) {
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
    auto packet{reset_packet};

    ESP_ERROR_CHECK(transmit_packet(packet));
    ESP_ERROR_CHECK(gpio_set_level(d21_gpio_num, d21_state = !d21_state));

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

    dcc_encoder_config_t encoder_config{.num_preamble = 17u,
                                        .bit1_duration = 58u,
                                        .bit0_duration = 100u,
                                        .endbit_duration = 58u - 24u};

    ESP_ERROR_CHECK(resume(encoder_config, rmt_callback, gptimer_callback));

    operations_loop();
  }
}

}  // namespace out::track::dcc