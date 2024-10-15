// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize ZUSI
///
/// \file   out/zusi/init.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <driver/spi_master.h>
#include <esp_err.h>
#include <array>

namespace out::zusi {

inline std::array<spi_device_handle_t, 4uz> spis{};

esp_err_t init(BaseType_t xCoreID);

}  // namespace out::zusi