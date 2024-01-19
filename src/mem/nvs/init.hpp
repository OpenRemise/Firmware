/// Initialize NVS
///
/// \file   mem/nvs/init.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_err.h>

namespace mem::nvs {

esp_err_t init();

}  // namespace mem::nvs