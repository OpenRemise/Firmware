// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize HTTP
///
/// \file   http/init.cpp
/// \author Vincent Hamp
/// \date   01/03/2023

#include "init.hpp"
#include <esp_wifi.h>
#include "ap/init.hpp"
#include "log.h"
#include "sta/init.hpp"

namespace http {

///
esp_err_t init() {
  wifi_mode_t mode;
  ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
  if (mode == WIFI_MODE_AP) return ap::init();
  else if (mode == WIFI_MODE_STA) return sta::init();
  else return ESP_FAIL;
}

}  // namespace http
