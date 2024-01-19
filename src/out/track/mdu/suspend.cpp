#include "suspend.hpp"
#include "../suspend.hpp"
#include "analog/init.hpp"

namespace out::track::mdu {

///
esp_err_t suspend() {
  ESP_ERROR_CHECK(track::suspend());
  return analog::init(analog::sample_freq_hz);
}

}  // namespace out::track::mdu