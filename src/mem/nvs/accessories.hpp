// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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