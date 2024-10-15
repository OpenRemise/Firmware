// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

///
///
/// \file   z21/init.hpp
/// \author Vincent Hamp
/// \date   15/04/2024

#pragma once

#include <esp_err.h>

namespace z21 {

esp_err_t init(BaseType_t xCoreID);

}  // namespace z21