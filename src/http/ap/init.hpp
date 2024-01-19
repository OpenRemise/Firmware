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