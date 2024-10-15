// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// USB receive task function
///
/// \file   usb/rx_task_function.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <string_view>

namespace usb {

bool any_protocol_task_active();
void rx_task_function(void*);

}  // namespace usb