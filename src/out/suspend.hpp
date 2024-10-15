// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// \todo document
///
/// \file   out/suspend.hpp
/// \author Vincent Hamp
/// \date   23/04/2023

#pragma once

#include <esp_err.h>

namespace out {

void reset_queue_and_message_buffers();
esp_err_t suspend();

}  // namespace out