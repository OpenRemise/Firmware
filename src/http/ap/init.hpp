// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize HTTP server for access point
///
/// \file   http/ap/init.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <esp_err.h>

namespace http::ap {

esp_err_t init();

}  // namespace http::ap