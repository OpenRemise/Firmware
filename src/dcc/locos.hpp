// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include "loco.hpp"

namespace dcc {

using Locos = std::map<Address::value_type, Loco>;

}  // namespace dcc