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

/// ZUSI task function
///
/// \file   out/zusi/task_function.cpp
/// \author Vincent Hamp
/// \date   27/03/2023

#include "task_function.hpp"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <hal/gpio_ll.h>
#include <soc/spi_periph.h>
#include <ulf/susiv2.hpp>
#include <zusi/zusi.hpp>
#include "init.hpp"
#include "log.h"
#include "resume.hpp"
#include "suspend.hpp"
#include "utility.hpp"

namespace out::zusi {

using namespace ::zusi;
using ::ulf::susiv2::ack, ::ulf::susiv2::nak;

class ZppLoad final : public ::zusi::tx::Base {
  /// \todo document
  void transmitBytes(std::span<uint8_t const> chunk,
                     ::zusi::Mbps mbps) const final {
    spi_transaction_t trans{
      .length = size(chunk) * CHAR_BIT,
      .tx_buffer = data(chunk),
    };
    ESP_ERROR_CHECK(
      spi_device_polling_transmit(spis[std::to_underlying(mbps)], &trans));
  }

  /// \todo document
  void spiMaster() const final {
    gpio_set_direction(data_gpio_num, GPIO_MODE_OUTPUT);
    esp_rom_gpio_connect_out_signal(
      data_gpio_num, spi_periph_signal[SPI2_HOST].spid_out, false, false);
    esp_rom_gpio_connect_out_signal(
      clock_gpio_num, spi_periph_signal[SPI2_HOST].spiclk_out, false, false);
  }

  /// \todo document
  void gpioInput() const final {
    gpio_set_direction(data_gpio_num, GPIO_MODE_INPUT);
    esp_rom_gpio_connect_out_signal(
      clock_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  }

  /// \todo document
  void gpioOutput() const final {
    gpio_set_direction(data_gpio_num, GPIO_MODE_OUTPUT);
    esp_rom_gpio_connect_out_signal(
      data_gpio_num, SIG_GPIO_OUT_IDX, false, false);
    esp_rom_gpio_connect_out_signal(
      clock_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  }

  /// \todo document
  void writeClock(bool state) const final {
    gpio_set_level(clock_gpio_num, state);
  }

  /// \todo document
  void writeData(bool state) const final {
    gpio_set_level(data_gpio_num, state);
  }

  /// \todo document
  bool readData() const final { return gpio_get_level(data_gpio_num); }

  /// \todo document
  void delayUs(uint32_t us) const final {
    gptimer_set_raw_count(gptimer, 0ull);
    uint64_t value{};
    while (value < us) ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &value));
  }
};

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
void transmit_response(ulf::susiv2::Response resp) {
  xMessageBufferSend(out::rx_message_buffer.handle, data(resp), size(resp), 0u);
}

/// \todo document
void loop() {
  // Give decoder some time to boot...
  vTaskDelay(pdMS_TO_TICKS(1000u));

  ZppLoad zpp_load;
  zpp_load.enter();

  while (auto const packet{receive_packet()}) {
    auto const fb{zpp_load.transmit(*packet)};
    auto const resp{ulf::susiv2::feedback2response(fb)};
    transmit_response(resp);
    if (static_cast<Command>((*packet)[0uz]) != Command::Exit) continue;
    vTaskDelay(pdMS_TO_TICKS(1000u));
    return;
  }
}

/// \todo document
void task_function(void*) {
  for (;;) switch (state.load()) {
      case State::ZUSI: [[fallthrough]];
      case State::ULF_SUSIV2:
        ESP_ERROR_CHECK(resume());
        loop();
        ESP_ERROR_CHECK(suspend());
        break;
      default: LOGI_TASK_SUSPEND(task.handle); break;
    }
}

} // namespace out::zusi