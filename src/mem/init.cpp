/// Initialize memories (NVS and SPIFFS)
///
/// \file   mem/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include "nvs/init.hpp"
#include "spiffs/init.hpp"

namespace mem {

///
esp_err_t init() {
  ESP_ERROR_CHECK(nvs::init());
  return spiffs::init();
}

}  // namespace mem