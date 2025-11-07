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

/// MDU task function
///
/// \file   drv/out/track/zimo/mdu/task_function.cpp
/// \author Vincent Hamp
/// \date   10/04/2024

#include "task_function.hpp"
#include <rmt_dcc_encoder.h>
#include <dcc/dcc.hpp>
#include <mdu/mdu.hpp>
#include <ulf/mdu_ein.hpp>
#include "../../current_limit.hpp"
#include "../../dcc/resume.hpp"
#include "../../dcc/suspend.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "resume.hpp"
#include "suspend.hpp"
#include "utility.hpp"

namespace drv::out::track::zimo::mdu {

using namespace ::mdu;
using ::ulf::mdu_ein::ack, ::ulf::mdu_ein::nak;

namespace {

/// \todo document
///
/// https://github.com/OpenRemise/Firmware/issues/52
uint32_t volatile gptimer_count_at_first_ack{};

/// \todo document
void IRAM_ATTR ack_isr_handler(void*) {
  uint64_t value{};
  gptimer_get_raw_count(gptimer, &value);
  if (!gptimer_count_at_first_ack)
    gptimer_count_at_first_ack = static_cast<uint32_t>(value);
}

/// \todo document
constexpr mdu_encoder_config_t mdu_encoder_config() {
  return {
    .transfer_rate = std::to_underlying(TransferRate::Default),
    .num_preamble = MDU_TX_MIN_PREAMBLE_BITS,
    .num_ackreq = MDU_TX_MIN_ACKREQ_BITS,
  };
}

/// \todo document
std::optional<Packet> receive_packet(uint32_t timeout) {
  Packet packet;
  //
  if (auto const bytes_received{
        xMessageBufferReceive(tx_message_buffer.front_handle,
                              data(packet),
                              packet.max_size(),
                              pdMS_TO_TICKS(timeout))}) {
    packet.resize(bytes_received);
    return packet;
  }
  //
  else
    return std::nullopt;
}

/// \todo document
esp_err_t transmit_packet_blocking(Packet const& packet) {
  static constexpr rmt_transmit_config_t config{};
  ESP_ERROR_CHECK(
    rmt_transmit(channel, encoder, data(packet), size(packet), &config));

  // Start timer
  ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0ull));

  // Clear any glitches
  gptimer_count_at_first_ack = 0u;

  // Wait
  return rmt_tx_wait_all_done(channel, -1);
}

/// \todo document
esp_err_t transmit_packet_blocking_for(Packet const& packet, uint32_t us) {
  auto const then{esp_timer_get_time() + us};
  while (esp_timer_get_time() < then)
    ESP_ERROR_CHECK(transmit_packet_blocking(packet));
  return ESP_OK;
}

/// \todo document
std::pair<int32_t, int32_t>
packet2ack_counts(mdu_encoder_config_t const& encoder_config,
                  Packet const& packet) {
  // Preamble
  auto count{static_cast<int32_t>(encoder_config.num_preamble *
                                  timings[encoder_config.transfer_rate].one)};

  // Start
  count += size(packet) * timings[encoder_config.transfer_rate].zero;

  // Data
  auto const ones{
    std::accumulate(cbegin(packet), cend(packet), 0uz, [](auto a, auto b) {
      return a + std::popcount(b);
    })};
  auto const zeros{size(packet) * CHAR_BIT - ones};
  count += ones * timings[encoder_config.transfer_rate].one +  // Ones
           zeros * timings[encoder_config.transfer_rate].zero; // Zeros

  // End
  count += timings[encoder_config.transfer_rate].one;

  return {count + 2u * timings[encoder_config.transfer_rate].ackreq,
          count + 6u * timings[encoder_config.transfer_rate].ackreq};
}

/// \todo document
std::array<uint8_t, 2uz>
receive_acks(mdu_encoder_config_t const& encoder_config, Packet const& packet) {
  // Get timer count
  auto const count{static_cast<int32_t>(gptimer_count_at_first_ack)};
  std::array<uint8_t, 2uz> acks{nak, nak};

  // ACK
  if (count) {
    auto const ack_counts{packet2ack_counts(encoder_config, packet)};
    std::pair const ack_diffs{std::abs(ack_counts.first - count),
                              std::abs(ack_counts.second - count)};
    if (ack_diffs.first < ack_diffs.second) acks[0uz] = ack;
    else acks[1uz] = ack;
  }

  return acks;
}

/// \todo document
esp_err_t transmit_acks(std::array<uint8_t, 2uz> acks) {
  return xMessageBufferSend(
           rx_message_buffer.handle, data(acks), size(acks), 0u) == size(acks)
           ? ESP_OK
           : ESP_FAIL;
}

/// \todo document
esp_err_t config_transfer_rate(mdu_encoder_config_t& encoder_config,
                               uint8_t transfer_rate,
                               std::array<uint8_t, 2uz> const& acks) {
  assert(transfer_rate <= std::to_underlying(TransferRate::Default));
  ESP_ERROR_CHECK(deinit_encoder());
  encoder_config.transfer_rate = acks[0uz] == ack || acks[1uz] == ack
                                   ? std::to_underlying(TransferRate::Fallback)
                                   : transfer_rate;
  ESP_ERROR_CHECK(init_encoder(encoder_config));
  return acks[0uz] == ack ? ESP_ERR_INVALID_CRC : ESP_OK;
}

/// \todo document
esp_err_t zpp_entry() {
  static constexpr rmt_transmit_config_t config{};

  mem::nvs::Settings nvs;
  dcc_encoder_config_t encoder_config{
    .num_preamble = DCC_TX_MAX_PREAMBLE_BITS,
    .bidibit_duration = 0u,
    .bit1_duration = nvs.getDccBit1Duration(),
    .bit0_duration = nvs.getDccBit0Duration(),
  };
  auto const program_packet_count{nvs.getDccProgramPacketCount()};
  nvs.~Settings();

  //
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_4100mA));

  //
  ESP_ERROR_CHECK(out::track::dcc::resume(encoder_config, nullptr, nullptr));

  // Transmit 500 idle packets (~4s)
  auto packet{::dcc::make_idle_packet()};
  for (auto i{0uz}; i < 500uz; ++i) {
    ESP_ERROR_CHECK(
      rmt_transmit(channel, encoder, data(packet), size(packet), &config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));
  }

  // CV verifies
  for (static constexpr std::array<std::pair<uint8_t, uint8_t>, 9uz> sequence{{
         {8u - 1u, 0xFEu},
         {105u - 1u, 0xAAu},
         {106u - 1u, 0x55u},
         {105u - 1u, 0x55u},
         {106u - 1u, 0xAAu},
         {105u - 1u, 0x00u},
         {106u - 1u, 0x00u},
         {105u - 1u, 0x00u},
         {106u - 1u, 0x00u},
       }};
       auto const& [cv_addr, byte] : sequence) {
    packet = ::dcc::make_cv_access_long_verify_packet(
      {.type = ::dcc::Address::Broadcast}, cv_addr, byte);
    for (auto i{0uz}; i < program_packet_count; ++i) {
      ESP_ERROR_CHECK(
        rmt_transmit(channel, encoder, data(packet), size(packet), &config));
      ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));
    }
  }

  //
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_500mA));

  //
  ESP_ERROR_CHECK(out::track::dcc::deinit_bidi());
  ESP_ERROR_CHECK(out::track::dcc::deinit_alarm());
  ESP_ERROR_CHECK(out::track::dcc::deinit_rmt());
  ESP_ERROR_CHECK(out::track::dcc::deinit_encoder());

  return ESP_OK;
}

/// \todo document
esp_err_t zsu_entry() {
  //
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_4100mA));

  //
  ESP_ERROR_CHECK(transmit_packet_blocking_for(make_busy_packet(), 200'000u));

  //
  ESP_ERROR_CHECK(set_current_limit(CurrentLimit::_500mA));

  return ESP_OK;
}

/// \todo document
esp_err_t loop(mdu_encoder_config_t& encoder_config) {
  auto const timeout{http_receive_timeout2ms()};
  auto const busy_packet{make_busy_packet()};

  for (;;) {
    // Suspend or short circuit
    if (std::to_underlying(state.load() &
                           (State::Suspending | State::ShortCircuit)))
      return rmt_tx_wait_all_done(channel, -1);
    // Timeout
    else if (auto const packet{receive_packet(task.timeout)}; !packet)
      return rmt_tx_wait_all_done(channel, -1);
    // Transmit packet
    else {
      ESP_ERROR_CHECK(transmit_packet_blocking(*packet));
      auto const acks{receive_acks(encoder_config, *packet)};
      ESP_ERROR_CHECK(transmit_acks(acks));

      // Transfer rate packet
      if (auto const cmd{packet2command(*packet)};
          cmd == Command::ConfigTransferRate)
        config_transfer_rate(encoder_config, (*packet)[4uz], acks);
      // Exit
      else if (cmd == Command::ZppExitReset ||
               cmd == Command::ZsuCrc32ResultExit)
        return transmit_packet_blocking_for(busy_packet, 1'000'000u);
    }
  }
}

/// \todo document that this pings a decoder (default MS450)
[[maybe_unused]] esp_err_t test_loop(mdu_encoder_config_t& encoder_config,
                                     uint8_t decoder_id = 6u) {
  auto const packet{make_ping_packet(decoder_id)};
  ESP_ERROR_CHECK(transmit_packet_blocking(packet));
  auto const acks{receive_acks(encoder_config, packet)};
  if (acks == std::array{nak, ack}) {
    LOGI("MDU test success");
    return ESP_OK;
  } else {
    LOGE("MDU test failure");
    return ESP_FAIL;
  }
}

} // namespace

/// \todo document
[[noreturn]] void task_function(void*) {
  switch (auto encoder_config{mdu_encoder_config()}; state.load()) {
    case State::MDUZpp: ESP_ERROR_CHECK(zpp_entry()); [[fallthrough]];
    case State::MDUZsu: [[fallthrough]];
    case State::ULF_MDU_EIN:
      ESP_ERROR_CHECK(resume(encoder_config, ack_isr_handler));
      ESP_ERROR_CHECK(zsu_entry());
      ESP_ERROR_CHECK(loop(encoder_config));
      ESP_ERROR_CHECK(suspend());
      break;
    default: assert(false); break;
  }
  LOGI_TASK_DESTROY();
}

} // namespace drv::out::track::zimo::mdu
