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

/// Initialize ZUSI
///
/// \file   out/zusi/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <driver/gpio.h>
#include "log.h"
#include "task_function.hpp"

namespace out::zusi {

/// \todo document
esp_err_t init() {
  static constexpr gpio_config_t io_conf{.pin_bit_mask = 1ull
                                                         << enable_gpio_num,
                                         .mode = GPIO_MODE_OUTPUT,
                                         .pull_up_en = GPIO_PULLUP_DISABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE};
  ESP_ERROR_CHECK(gpio_config(&io_conf));

  static constexpr spi_bus_config_t buscfg{.mosi_io_num = data_gpio_num,
                                           .miso_io_num = -1,
                                           .sclk_io_num = clock_gpio_num,
                                           .data2_io_num = -1,
                                           .data3_io_num = -1,
                                           .data4_io_num = -1,
                                           .data5_io_num = -1,
                                           .data6_io_num = -1,
                                           .data7_io_num = -1};

  // Can't use SPI0 or 1
  spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

  spi_device_interface_config_t devcfg{
    .command_bits = 0u,
    .address_bits = 0u,
    .dummy_bits = 0u,
    .mode = 1u, // Fuck the docs, mode ain't 3...
    .duty_cycle_pos = 0u,
    .cs_ena_pretrans = 0u,
    .cs_ena_posttrans = 0u,
    .clock_speed_hz = static_cast<int>(1.0 / 10e-6),
    .input_delay_ns = 0,
    .spics_io_num = -1,
    .flags = SPI_DEVICE_BIT_LSBFIRST | SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE,
    .queue_size = 7};
  spi_bus_add_device(SPI2_HOST, &devcfg, &spis[0uz]); // Fallback and resync

  devcfg.clock_speed_hz = static_cast<int>(1.0 / 3.5e-6);
  spi_bus_add_device(SPI2_HOST, &devcfg, &spis[1uz]);

  devcfg.clock_speed_hz = static_cast<int>(1.0 / 0.733e-6);
  spi_bus_add_device(SPI2_HOST, &devcfg, &spis[2uz]);

  devcfg.clock_speed_hz = static_cast<int>(1.0 / 0.5533e-6);
  spi_bus_add_device(SPI2_HOST, &devcfg, &spis[3uz]);

  task.create(task_function);

  return ESP_OK;
}

} // namespace out::zusi
