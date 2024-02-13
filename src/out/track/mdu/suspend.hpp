#pragma once

#include <esp_err.h>
#include "../suspend.hpp"

namespace out::track::mdu {

using track::deinit_encoder;
esp_err_t suspend();

}  // namespace out::track::mdu