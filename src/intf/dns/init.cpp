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

/// Initialize DNS
///
/// \file   intf/dns/init.cpp
/// \author Vincent Hamp
/// \date   19/10/2025

#include "init.hpp"
#include <dns_server.h>
#include <esp_wifi.h>
#include "mem/nvs/settings.hpp"

namespace intf::dns {

/// Initialize DNS
///
/// Initialization takes place in init(). In access point mode it creates a DNS
/// server and responds to all queries with the address of the softAP. This
/// "funnelling" of DNS and traffic triggers the captive portal (sign in) to
/// appear.
///
/// \warning
/// The DNS server will **not** redirect HTTPS requests.
esp_err_t init() {
  // Only start DNS in AP mode
  wifi_mode_t mode;
  ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));

  // Start the DNS server that will redirect all queries to the softAP IP
  if (mode == WIFI_MODE_AP) {
    dns_server_config_t config =
      DNS_SERVER_CONFIG_SINGLE("*",            // all A queries
                               "WIFI_AP_DEF"); // softAP netif ID

    start_dns_server(&config);
  }

  return ESP_OK;
}

} // namespace intf::dns
