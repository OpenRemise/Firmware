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

/// Initialize SUSI
///
/// \file   drv/out/susi/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include "log.h"
#include "zimo/zusi/task_function.hpp"

namespace drv::out::susi {

/// \todo document
esp_err_t init() {
  static constexpr gpio_config_t gpio_cfg{.pin_bit_mask = 1ull
                                                          << enable_gpio_num,
                                          .mode = GPIO_MODE_OUTPUT,
                                          .pull_up_en = GPIO_PULLUP_DISABLE,
                                          .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                          .intr_type = GPIO_INTR_DISABLE};
  ESP_ERROR_CHECK(gpio_config(&gpio_cfg));

  static constexpr spi_bus_config_t bus_cfg{.mosi_io_num = data_gpio_num,
                                            .miso_io_num = -1,
                                            .sclk_io_num = clock_gpio_num,
                                            .data2_io_num = -1,
                                            .data3_io_num = -1,
                                            .data4_io_num = -1,
                                            .data5_io_num = -1,
                                            .data6_io_num = -1,
                                            .data7_io_num = -1};

  // Can't use SPI1
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

  spi_device_interface_config_t dev_cfg{
    .command_bits = 0u,
    .address_bits = 0u,
    .dummy_bits = 0u,
    .mode = 1u,
    .duty_cycle_pos = 0u,
    .cs_ena_pretrans = 0u,
    .cs_ena_posttrans = 0u,
    .clock_speed_hz = static_cast<int>(1.0 / 10e-6),
    .input_delay_ns = 0,
    .spics_io_num = -1,
    .flags = SPI_DEVICE_BIT_LSBFIRST | SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE,
    .queue_size = 7};
  ESP_ERROR_CHECK(
    spi_bus_add_device(SPI2_HOST, &dev_cfg, &spis[0uz])); // Fallback and resync

  dev_cfg.clock_speed_hz = static_cast<int>(1.0 / 3.5e-6);
  ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &spis[1uz]));

  dev_cfg.clock_speed_hz = static_cast<int>(1.0 / 0.733e-6);
  ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &spis[2uz]));

  dev_cfg.clock_speed_hz = static_cast<int>(1.0 / 0.5533e-6);
  ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &spis[3uz]));

  zimo::zusi::task.function = zimo::zusi::task_function;

  return ESP_OK;
}

} // namespace drv::out::susi
