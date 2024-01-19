/// Initialize low level layer (ADC, track and ZUSI)
///
/// \file   out/init.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_err.h>

namespace out {

esp_err_t init();

}  // namespace out