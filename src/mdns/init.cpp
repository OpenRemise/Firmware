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

/// Initialize mDNS
///
/// \file   mdns/init.cpp
/// \author Vincent Hamp
/// \date   21/03/2025

#include "init.hpp"
#include <esp_wifi.h>
#include <mdns.h>
#include "mem/nvs/settings.hpp"

namespace mdns {

/// \todo document
esp_err_t init() {
  ESP_ERROR_CHECK(mdns_init());

  mem::nvs::Settings nvs;
  auto const sta_mdns_str{nvs.getStationmDNS()};

  // STA mode: hostname is user setting
  // AP mode:  hostname fixed to "remise"
  wifi_mode_t mode;
  ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
  str = mode == WIFI_MODE_STA && !empty(sta_mdns_str) ? sta_mdns_str : "remise";
  ESP_ERROR_CHECK(mdns_hostname_set(str.c_str()));

  // Add services
  ESP_ERROR_CHECK(mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0));
  ESP_ERROR_CHECK(mdns_service_add(NULL, "_z21", "_udp", 21105, NULL, 0));

  return ESP_OK;
}

} // namespace mdns
