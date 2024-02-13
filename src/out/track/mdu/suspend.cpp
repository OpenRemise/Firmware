#include "suspend.hpp"
#include <driver/gpio.h>
#include "../suspend.hpp"

namespace out::track::mdu {

namespace {

///
esp_err_t deinit_gpio() {
  ESP_ERROR_CHECK(gpio_isr_handler_remove(ack_gpio_num));
  return gpio_install_isr_service(ESP_INTR_FLAG_INTRDISABLED);
}

}  // namespace

///
esp_err_t suspend() {
  ESP_ERROR_CHECK(deinit_gpio());
  return track::suspend();
}

}  // namespace out::track::mdu