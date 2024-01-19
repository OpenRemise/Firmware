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