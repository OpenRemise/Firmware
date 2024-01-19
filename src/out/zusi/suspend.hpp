/// Deinitialize peripherals when suspending ZUSI task
///
/// \file   out/zusi/suspend.hpp
/// \author Vincent Hamp
/// \date   27/03/2023

#pragma once

#include <esp_err.h>

namespace out::zusi {

esp_err_t suspend();

}  // namespace out::zusi