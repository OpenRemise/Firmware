/// Initialize peripherals when resuming ZUSI task
///
/// \file   out/zusi/resume.hpp
/// \author Vincent Hamp
/// \date   27/03/2023

#pragma once

#include <esp_err.h>

namespace out::zusi {

esp_err_t resume();

}  // namespace out::zusi