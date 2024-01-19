/// TODO
///
/// \file   out/track/suspend.hpp
/// \author Vincent Hamp
/// \date   04/07/2023

#pragma once

#include <esp_err.h>

namespace out::track {

esp_err_t deinit_encoder();
esp_err_t suspend();

}  // namespace out::track