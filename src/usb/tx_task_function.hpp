// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// USB transmit task function
///
/// \file   usb/tx_task_function.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

namespace usb {

void transmit_ok();
void transmit_not_ok();
void tx_task_function(void*);

}  // namespace usb