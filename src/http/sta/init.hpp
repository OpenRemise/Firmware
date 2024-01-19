/// Initialize HTTP server for station
///
/// \file   sta/init.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <esp_err.h>

namespace http::sta {

esp_err_t init();

}  // namespace http::sta