/// Initialize memories (NVS and SPIFFS)
///
/// \file   mem/init.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_err.h>

namespace mem {

esp_err_t init();

}  // namespace mem