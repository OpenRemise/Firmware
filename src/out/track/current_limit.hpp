// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

///
///
/// \file   out/track/current_limit.hpp
/// \author Vincent Hamp
/// \date   15/02/2024

#pragma once

#include <esp_err.h>

namespace out::track {

CurrentLimit get_current_limit();
esp_err_t set_current_limit(CurrentLimit current_limit);

}  // namespace out::track