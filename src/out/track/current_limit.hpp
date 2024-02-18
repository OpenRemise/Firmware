///
///
/// \file   out/track/current_limit.hpp
/// \author Vincent Hamp
/// \date   15/02/2024

#pragma once

#include <esp_err.h>

namespace out::track {

CurrentLimit get_current_limit();
esp_err_t set_current_limit(CurrentLimit current_limit);

}  // namespace out::track