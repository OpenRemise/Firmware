/// Initialize peripherals when resuming ZUSI task
///
/// \file   out/zusi/resume.cpp
/// \author Vincent Hamp
/// \date   27/03/2023

#include "resume.hpp"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <algorithm>
#include "../resume.hpp"
#include "init.hpp"

namespace out::zusi {

namespace {

///
esp_err_t init_gpio() { return gpio_set_level(enable_gpio_num, 1u); }

}  // namespace

///
esp_err_t resume() {
  ESP_ERROR_CHECK(out::resume(nullptr));
  return init_gpio();
}

}  // namespace out::zusi
