// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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