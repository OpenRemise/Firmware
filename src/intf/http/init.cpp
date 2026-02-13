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

/// Initialize HTTP
///
/// \file   intf/http/init.cpp
/// \author Vincent Hamp
/// \date   01/03/2023

#include "init.hpp"
#include <esp_wifi.h>
#include "ap/init.hpp"
#include "log.h"
#include "sta/init.hpp"

namespace intf::http {

/// \todo document
esp_err_t init() {
  wifi_mode_t mode{WIFI_MODE_NULL};
  esp_wifi_get_mode(&mode);
  return mode == WIFI_MODE_AP ? ap::init() : sta::init();
}

} // namespace intf::http
