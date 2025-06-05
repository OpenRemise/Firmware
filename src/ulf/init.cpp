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
/// \file   ulf/init.cpp
/// \author Vincent Hamp
/// \date   04/05/2025

#include "init.hpp"
#include <array>
#include "dcc_ein/task_function.hpp"
#include "decup_ein/task_function.hpp"
#include "log.h"
#include "susiv2/task_function.hpp"

namespace ulf {

/// \todo document
///
/// - ULF_DCC_EIN
/// - ULF_DECUP_EIN
/// - ULF_SUSIV2
esp_err_t init(BaseType_t xCoreID) {
  if (!xTaskCreatePinnedToCore(dcc_ein::task_function,
                               dcc_ein::task.name,
                               dcc_ein::task.stack_size,
                               NULL,
                               dcc_ein::task.priority,
                               &dcc_ein::task.handle,
                               xCoreID))
    assert(false);
  if (!xTaskCreatePinnedToCore(decup_ein::task_function,
                               decup_ein::task.name,
                               decup_ein::task.stack_size,
                               NULL,
                               decup_ein::task.priority,
                               &decup_ein::task.handle,
                               xCoreID))
    assert(false);
  if (!xTaskCreatePinnedToCore(susiv2::task_function,
                               susiv2::task.name,
                               susiv2::task.stack_size,
                               NULL,
                               susiv2::task.priority,
                               &susiv2::task.handle,
                               xCoreID))
    assert(false);

  return ESP_OK;
}

} // namespace ulf
