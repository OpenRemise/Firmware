/// Trace (IO pins to toggle for debug purposes)
///
/// \file   trace.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_err.h>

namespace trace {

esp_err_t init();

}  // namespace trace