// Copyright (C) 2024 Vincent Hamp
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

/// Initialize SPIFFS
///
/// \file   mem/spiffs/init.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "init.hpp"
#include <esp_spiffs.h>

namespace mem::spiffs {

///
esp_err_t init() {
  static constexpr esp_vfs_spiffs_conf_t conf{.base_path = "",
                                              .partition_label = NULL,
                                              .max_files = 5,
                                              .format_if_mount_failed = true};

  // Use settings defined above to initialize and mount SPIFFS filesystem.
  // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
  return esp_vfs_spiffs_register(&conf);
}

}  // namespace mem::spiffs