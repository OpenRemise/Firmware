// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize WiFi and mDNS
///
/// \file   wifi/init.hpp
/// \author Vincent Hamp
/// \date   02/07/2023

#pragma once

#include <esp_err.h>

namespace wifi {

esp_err_t init();

}  // namespace wifi