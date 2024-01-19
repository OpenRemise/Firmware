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

esp_err_t init();

}  // namespace out::zusi