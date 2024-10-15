// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Deinitialize peripherals when suspending MDU task
///
/// \file   out/track/mdu/suspend.hpp
/// \author Vincent Hamp
/// \date   10/04/2024

#pragma once

#include <esp_err.h>

namespace out::track::mdu {

esp_err_t deinit_encoder();
esp_err_t suspend();

}  // namespace out::track::mdu