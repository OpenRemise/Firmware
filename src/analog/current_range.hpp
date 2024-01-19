///
///
/// \file   analog/current_range.hpp
/// \author Vincent Hamp
/// \date   03/05/2023

#pragma once

#include <cstdint>

namespace analog {

enum class CurrentRange : uint8_t { Invalid, High, Low };

}  // namespace analog