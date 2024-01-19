/// NVS "accessories" namespace
///
/// \file   mem/nvs/accessories.hpp
/// \author Vincent Hamp
/// \date   17/02/2023

#pragma once

#include <dcc/dcc.hpp>
#include "base.hpp"

namespace mem::nvs {

class Accessories : public Base {
public:
  explicit Accessories() : Base{"accessories", NVS_READWRITE} {}
};

}  // namespace mem::nvs