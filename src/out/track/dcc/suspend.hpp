/// Deinitialize peripherals when suspending DCC task
///
/// \file   out/track/dcc/suspend.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <esp_err.h>

namespace out::track::dcc {

esp_err_t deinit_gpio();
esp_err_t deinit_bidi();
esp_err_t deinit_alarm();
esp_err_t deinit_rmt();
esp_err_t deinit_encoder();
esp_err_t suspend();

}  // namespace out::track::dcc