// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize peripherals when resuming ZUSI task
///
/// \file   out/zusi/resume.hpp
/// \author Vincent Hamp
/// \date   27/03/2023

#pragma once

#include <esp_err.h>

namespace out::zusi {

esp_err_t resume();

}  // namespace out::zusi