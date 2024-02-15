///
///
/// \file   out/track/current_limit.hpp
/// \author Vincent Hamp
/// \date   15/02/2024

#pragma once

#include <esp_err.h>

namespace out::track {

enum class CurrentLimit {
  _500 = 0b00u,
  _1600 = 0b01u,
  _3000 = 0b10u,
  _4100 = 0b11u
};

CurrentLimit get_current_limit();
esp_err_t set_current_limit(CurrentLimit current_limit);

}  // namespace out::track