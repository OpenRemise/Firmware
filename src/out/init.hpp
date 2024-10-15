// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize low level layer (ADC, track and ZUSI)
///
/// \file   out/init.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_err.h>

namespace out {

esp_err_t init(BaseType_t xCoreID);

}  // namespace out