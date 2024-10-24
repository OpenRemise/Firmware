// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize NVS
///
/// \file   mem/nvs/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <nvs_flash.h>

namespace mem::nvs {

/// Initialize NVS
esp_err_t init() {
  auto err{nvs_flash_init()};

  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    ESP_ERROR_CHECK(nvs_flash_erase());
    // Retry nvs_flash_init
    err = nvs_flash_init();
  }

  return err;
}

}  // namespace mem::nvs
