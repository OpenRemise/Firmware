// Copyright (C) 2025 Vincent Hamp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/// Initialize ULF
///
/// \file   mw/zimo/ulf/init.cpp
/// \author Vincent Hamp
/// \date   04/05/2025

#include "init.hpp"
#include <array>
#include "dcc_ein/task_function.hpp"
#include "log.h"
#include "susiv2/task_function.hpp"

namespace mw::zimo::ulf {

/// Initialize ULF
///
/// This function basically does nothing except set the function pointer for the
/// corresponding protocol tasks:
/// - [ULF_DCC_EIN](https://github.com/ZIMO-Elektronik/ULF_DCC_EIN)
/// - [ULF_SUSIV2](https://github.com/ZIMO-Elektronik/ULF_SUSIV2)
esp_err_t init() {
  dcc_ein::task.function = dcc_ein::task_function;
  susiv2::task.function = susiv2::task_function;
  return ESP_OK;
}

} // namespace mw::zimo::ulf
