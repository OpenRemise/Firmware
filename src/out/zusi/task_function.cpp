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
  ///
  void transmitBytes(std::span<uint8_t const> chunk,
                     ::zusi::Mbps mbps) const final {
    spi_transaction_t trans{
      .length = size(chunk) * CHAR_BIT,
      .tx_buffer = data(chunk),
    };
    ESP_ERROR_CHECK(
      spi_device_polling_transmit(spis[std::to_underlying(mbps)], &trans));
  }

  ///
  void spiMaster() const final {
    gpio_set_direction(data_gpio_num, GPIO_MODE_OUTPUT);
    esp_rom_gpio_connect_out_signal(
      data_gpio_num, spi_periph_signal[SPI2_HOST].spid_out, false, false);
    esp_rom_gpio_connect_out_signal(
      clock_gpio_num, spi_periph_signal[SPI2_HOST].spiclk_out, false, false);
  }

  ///
  void gpioInput() const final {
    gpio_set_direction(data_gpio_num, GPIO_MODE_INPUT);
    esp_rom_gpio_connect_out_signal(
      clock_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  }

  ///
  void gpioOutput() const final {
    gpio_set_direction(data_gpio_num, GPIO_MODE_OUTPUT);
    esp_rom_gpio_connect_out_signal(
      data_gpio_num, SIG_GPIO_OUT_IDX, false, false);
    esp_rom_gpio_connect_out_signal(
      clock_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  }

  ///
  void writeClock(bool state) const final {
    gpio_set_level(clock_gpio_num, state);
  }

  ///
  void writeData(bool state) const final {
    gpio_set_level(data_gpio_num, state);
  }

  ///
  bool readData() const final { return gpio_get_level(data_gpio_num); }

  ///
  void delayUs(uint32_t us) const final {
    gptimer_set_raw_count(gptimer, 0ull);
    uint64_t value{};
    while (value < us) ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &value));
  }
};

/// \todo document
std::optional<std::span<uint8_t>> receive_command(std::span<uint8_t> stack) {
  //
  if (auto const bytes_received{
        xMessageBufferReceive(tx_message_buffer.front_handle,
                              data(stack),
                              size(stack),
                              pdMS_TO_TICKS(http_receive_timeout2ms()))})
    return stack.subspan(0uz, bytes_received);
  //
  else return std::nullopt;
}

/// \todo document
void transmit(std::span<uint8_t const> stack) {
  xMessageBufferSend(
    out::rx_message_buffer.handle, data(stack), size(stack), 0u);
}

/// \todo document
void loop() {
  ::zusi::Buffer<tx_message_buffer.size> stack;

  // Give decoder some time to boot...
  vTaskDelay(pdMS_TO_TICKS(1000u));

  ZppLoad zpp_load;
  zpp_load.enter();

  while (auto const cmd{receive_command(stack)}) {
    auto const i{data2uint32(&stack[2uz])};

    switch (static_cast<Command>(stack[0uz])) {
      case Command::None: break;

      case Command::CvRead:
        if (auto const cv{zpp_load.readCv(i)}) {
          stack[0uz] = ack;
          stack[1uz] = *cv;
          stack[2uz] = crc8(stack[1uz]);
          transmit({cbegin(stack), 3uz});
        } else transmit({&nak, 1uz});
        break;

      case Command::CvWrite:
        stack[0uz] = zpp_load.writeCv(i, stack[6uz]) ? ack : nak;
        transmit({cbegin(stack), 1uz});
        break;

      case Command::ZppErase:
        stack[0uz] = zpp_load.eraseZpp() ? ack : nak;
        transmit({cbegin(stack), 1uz});
        break;

      case Command::ZppWrite:
        stack[0uz] = zpp_load.writeZpp(i, {&stack[6uz], 256uz}) ? ack : nak;
        transmit({cbegin(stack), 1uz});
        break;

      case Command::Features:
        if (auto const features{zpp_load.features()}) {
          auto first{begin(stack)};
          auto last{first};
          *last++ = ack;
          last = std::copy(cbegin(*features), cend(*features), last);
          *last = crc8({first, last});
          transmit({first, 6uz});
        } else transmit({&nak, 1uz});
        break;

      case Command::Exit:
        stack[0uz] = zpp_load.exit(stack[3uz]) ? ack : nak;
        transmit({cbegin(stack), 1uz});
        if (stack[0uz] == ack) {
          vTaskDelay(pdMS_TO_TICKS(1000u));
          return;
        }
        break;

      /// \todo implement encrypt
      case Command::Encrypt: LOGW("'Encrypt' command not implemented"); break;
    }
  }
}

/// \todo document
void task_function(void*) {
  for (;;) switch (state.load()) {
      case State::SUSIV2: [[fallthrough]];
      case State::ZUSI:
        ESP_ERROR_CHECK(resume());
        loop();
        ESP_ERROR_CHECK(suspend());
        break;
      default: LOGI_TASK_SUSPEND(task.handle); break;
    }
}

}  // namespace out::zusi