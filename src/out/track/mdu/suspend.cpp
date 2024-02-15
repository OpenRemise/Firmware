#include "suspend.hpp"
#include <driver/gpio.h>
#include "../suspend.hpp"

namespace out::track::mdu {

namespace {

///
esp_err_t deinit_gpio() { return gpio_isr_handler_remove(ack_gpio_num); }

}  // namespace

///
esp_err_t suspend() {
  ESP_ERROR_CHECK(deinit_gpio());
  return track::suspend();
}

}  // namespace out::track::mdu