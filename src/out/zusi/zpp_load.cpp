///
///
/// \file   out/zusi/zpp_load.cpp
/// \author Vincent Hamp
/// \date   31/03/2023

#include "zpp_load.hpp"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <hal/gpio_ll.h>
#include <soc/spi_periph.h>
#include "init.hpp"

namespace out::zusi {

void ZppLoad::transmitBytes(std::span<uint8_t const> chunk,
                            ::zusi::Mbps mbps) const {
  spi_transaction_t trans{
    .length = size(chunk) * CHAR_BIT,
    .tx_buffer = data(chunk),
  };
  ESP_ERROR_CHECK(
    spi_device_polling_transmit(spis[std::to_underlying(mbps)], &trans));
}

///
void ZppLoad::spiMaster() const {
  gpio_set_direction(data_gpio_num, GPIO_MODE_OUTPUT);
  esp_rom_gpio_connect_out_signal(
    data_gpio_num, spi_periph_signal[SPI2_HOST].spid_out, false, false);
  esp_rom_gpio_connect_out_signal(
    clock_gpio_num, spi_periph_signal[SPI2_HOST].spiclk_out, false, false);
}

///
void ZppLoad::gpioInput() const {
  gpio_set_direction(data_gpio_num, GPIO_MODE_INPUT);
  esp_rom_gpio_connect_out_signal(
    clock_gpio_num, SIG_GPIO_OUT_IDX, false, false);
}

///
void ZppLoad::gpioOutput() const {
  gpio_set_direction(data_gpio_num, GPIO_MODE_OUTPUT);
  esp_rom_gpio_connect_out_signal(
    data_gpio_num, SIG_GPIO_OUT_IDX, false, false);
  esp_rom_gpio_connect_out_signal(
    clock_gpio_num, SIG_GPIO_OUT_IDX, false, false);
}

///
void ZppLoad::writeClock(bool state) const {
  gpio_set_level(clock_gpio_num, state);
}

///
void ZppLoad::writeData(bool state) const {
  gpio_set_level(data_gpio_num, state);
}

///
bool ZppLoad::readData() const { return gpio_get_level(data_gpio_num); }

///
void ZppLoad::delayUs(uint32_t us) const {
  gptimer_set_raw_count(gptimer, 0ull);
  uint64_t value{};
  while (value < us) ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &value));
}

}  // namespace out::zusi