/// Trace (IO pins to toggle for debug purposes)
///
/// \file   trace.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "trace.hpp"
#include <driver/gpio.h>

namespace trace {

///
esp_err_t init() {
  //
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << d21_gpio_num | 1ull << d20_gpio_num,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&io_conf));
  }

  //
  {
    static constexpr gpio_config_t io_conf{
      .pin_bit_mask = 1ull << bug_led_gpio_num,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE};
    return gpio_config(&io_conf);
  }
}

}  // namespace trace