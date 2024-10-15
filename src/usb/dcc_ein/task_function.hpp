// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// DCC_EIN protocol task function
///
/// \file   usb/dcc_ein/task_function.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <optional>
#include <span>

namespace usb::dcc_ein {

std::optional<dcc::Packet> receive_dcc_packet();
void task_function(void*);

}  // namespace usb::dcc_ein