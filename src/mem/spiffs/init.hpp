/// Initialize SPIFFS
///
/// \file   mem/spiffs/init.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <esp_err.h>

namespace mem::spiffs {

esp_err_t init();

}  // namespace mem::spiffs