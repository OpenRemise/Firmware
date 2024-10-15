// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Deinitialize peripherals when suspending DECUP task
///
/// \file   out/track/decup/suspend.hpp
/// \author Vincent Hamp
/// \date   14/08/2024

#pragma once

#include <esp_err.h>

namespace out::track::decup {

esp_err_t deinit_encoder();
esp_err_t suspend();

}  // namespace out::track::decup