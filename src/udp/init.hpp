// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize UDP
///
/// \file   udp/init.hpp
/// \author Vincent Hamp
/// \date   04/04/2024

#pragma once

#include <esp_err.h>

namespace udp {

esp_err_t init();

}  // namespace udp