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

/// Initialize WiFi
///
/// \file   drv/wifi/init.cpp
/// \author Vincent Hamp
/// \date   02/07/2023

#include "init.hpp"
#include <driver/gpio.h>
#include <esp_mac.h>
#include <esp_wifi.h>
#include <bit>
#include <cstring>
#include <optional>
#include <vector>
#include <ztl/string.hpp>
#include "drv/led/wifi.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"

namespace drv::wifi {

namespace {

/// \todo document
void event_handler(void*,
                   esp_event_base_t event_base,
                   int32_t event_id,
                   void* event_data) {
  static size_t connection_count{};

  // Station got IP from connected AP
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    auto const event{static_cast<ip_event_got_ip_t*>(event_data)};
    auto const count{
      snprintf(data(ip), size(ip), IPSTR, IP2STR(&event->ip_info.ip))};
    ip_str.replace(0uz, count, data(ip));
    led::wifi(++connection_count);
    LOGI("IP_EVENT_STA_GOT_IP %s", ip_str.c_str());
  }
  // Station lost IP and the IP is reset to 0
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
    ip.fill(0);
    ip_str.clear();
    led::wifi((connection_count -= connection_count != 0uz));
    esp_wifi_connect();
    LOGI("IP_EVENT_STA_LOST_IP");
  }
  // Station connected to AP
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    auto const event{static_cast<wifi_event_sta_connected_t*>(event_data)};
    LOGI("WIFI_EVENT_STA_CONNECTED %.*s", event->ssid_len, event->ssid);
  }
  // Station disconnected from AP
  else if (event_base == WIFI_EVENT &&
           event_id == WIFI_EVENT_STA_DISCONNECTED) {
    auto const event{static_cast<wifi_event_sta_disconnected_t*>(event_data)};
    ip.fill(0);
    ip_str.clear();
    led::wifi((connection_count -= connection_count != 0uz));
    esp_wifi_connect();
    LOGI("WIFI_EVENT_STA_DISCONNECTED %.*s", event->ssid_len, event->ssid);
  }
  // Station connected to Soft-AP
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
    [[maybe_unused]] auto const event{
      static_cast<wifi_event_ap_staconnected_t*>(event_data)};
    led::wifi(++connection_count);
    LOGI("WIFI_EVENT_AP_STACONNECTED");
  }
  // Station disconnected from Soft-AP
  else if (event_base == WIFI_EVENT &&
           event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    [[maybe_unused]] auto const event{
      static_cast<wifi_event_ap_stadisconnected_t*>(event_data)};
    led::wifi((connection_count -= connection_count != 0uz));
    LOGI("WIFI_EVENT_AP_STADISCONNECTED");
  }
}

/// \todo document
wifi_ap_config_t ap_config() {
  wifi_ap_config_t ap{};
  mem::nvs::Settings nvs;

  // Read SSID
  auto const ssid{nvs.getAccessPointSSID()};
  memcpy(ap.ssid, data(ssid), size(ssid));
  ap.ssid_len = static_cast<uint8_t>(size(ssid));

  // Read password
  if (auto const pass{nvs.getAccessPointPassword()}; size(pass)) {
    memcpy(ap.password, data(pass), size(pass));
    ap.authmode = WIFI_AUTH_WPA2_PSK;
  } else ap.authmode = WIFI_AUTH_OPEN;

  //
  ap.max_connection = 4u;

  return ap;
}

/// \todo document
std::pair<wifi_sta_config_t, wifi_sta_config_t> sta_configs() {
  std::pair<wifi_sta_config_t, wifi_sta_config_t> stas{};
  mem::nvs::Settings nvs;

  // Read SSID
  auto const sta_ssid{nvs.getStationSSID()};
  memcpy(stas.first.ssid, data(sta_ssid), size(sta_ssid));

  // Read password
  auto const sta_pass{nvs.getStationPassword()};
  memcpy(stas.first.password, data(sta_pass), size(sta_pass));

  // Read alternative SSID
  auto const sta_alt_ssid{nvs.getStationAlternativeSSID()};
  memcpy(stas.second.ssid, data(sta_alt_ssid), size(sta_alt_ssid));

  // Read alternative password
  auto const sta_alt_pass{nvs.getStationAlternativePassword()};
  memcpy(stas.second.password, data(sta_alt_pass), size(sta_alt_pass));

  // Allow connecting to open networks
  stas.first.threshold.authmode = WIFI_AUTH_OPEN;
  stas.second.threshold.authmode = WIFI_AUTH_OPEN;

  return stas;
}

/// \todo document
std::pair<esp_netif_t*, esp_netif_t*>
wifi_init(wifi_ap_config_t const& ap_cfg,
          std::pair<wifi_sta_config_t, wifi_sta_config_t> const& sta_cfgs) {
  // Initialize TCP/IP stack and event loop
  esp_netif_init();
  esp_event_loop_create_default();

  // Register event handlers
  ESP_ERROR_CHECK(esp_event_handler_register(
    IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    IP_EVENT, IP_EVENT_STA_LOST_IP, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &event_handler, NULL));

  // Init WiFi driver to default and install it
  wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

  // Disable power saving
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

  return {ap_cfg.ssid[0uz] ? esp_netif_create_default_wifi_ap() : nullptr,
          sta_cfgs.first.ssid[0uz] ? esp_netif_create_default_wifi_sta()
                                   : nullptr};
}

/// Scan access points and sort by RSSI
esp_err_t scan_ap_records() {
  ESP_ERROR_CHECK(esp_wifi_start());
  esp_wifi_scan_start(NULL, true);
  uint16_t ap_count{};
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
  ap_records.resize(ap_count);
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, data(ap_records)));
  std::ranges::sort(
    ap_records, [](auto const& a, auto const& b) { return a.rssi > b.rssi; });
  return ESP_OK;
}

/// \todo document
esp_err_t init_ap(esp_netif_t* ap_netif, wifi_ap_config_t const& ap_cfg) {
  ESP_ERROR_CHECK(esp_netif_set_default_netif(ap_netif));
  wifi_config_t wifi_cfg{.ap = ap_cfg};
  return esp_wifi_set_config(WIFI_IF_AP, &wifi_cfg);
}

/// \todo document
esp_err_t
init_sta(esp_netif_t* sta_netif,
         std::pair<wifi_sta_config_t, wifi_sta_config_t> const& sta_cfgs) {
  // Pick the configuration with the best RSSI
  wifi_config_t wifi_cfg{};
  auto first{cbegin(ap_records)};
  auto const last{cend(ap_records)};
  while (first < last) {
    if (!memcmp(first->ssid, sta_cfgs.first.ssid, MAX_SSID_LEN)) {
      wifi_cfg.sta = sta_cfgs.first;
      break;
    } else if (!memcmp(first->ssid, sta_cfgs.second.ssid, MAX_SSID_LEN)) {
      wifi_cfg.sta = sta_cfgs.second;
      break;
    }
    ++first;
  }
  if (first == last) return ESP_FAIL;

  ESP_ERROR_CHECK(esp_netif_set_default_netif(sta_netif));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));

  // Static IP
  mem::nvs::Settings nvs;
  auto const sta_ip{nvs.getStationIP()};
  auto const sta_netmask{nvs.getStationNetmask()};
  auto const sta_gateway{nvs.getStationGateway()};
  if (!empty(sta_ip) && !empty(sta_netmask) && !empty(sta_gateway)) {
    esp_netif_ip_info_t ip_info;
    ESP_ERROR_CHECK(esp_netif_str_to_ip4(sta_ip.c_str(), &ip_info.ip));
    ESP_ERROR_CHECK(
      esp_netif_str_to_ip4(sta_netmask.c_str(), &ip_info.netmask));
    ESP_ERROR_CHECK(esp_netif_str_to_ip4(sta_gateway.c_str(), &ip_info.gw));
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(sta_netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(sta_netif, &ip_info));
  }

  return esp_wifi_connect();
}

} // namespace

/// Initialize either
/// - STA (station) if NVS contains SSID/pass or
/// - AP (access point) if NVS doesn't contain SSID/pass or network not found
esp_err_t init() {
  auto const ap_cfg{ap_config()};
  auto const sta_cfgs{sta_configs()};
  auto const [ap_netif, sta_netif]{(wifi_init(ap_cfg, sta_cfgs))};
  ESP_ERROR_CHECK(scan_ap_records());
  if (ap_netif) init_ap(ap_netif, ap_cfg);
  if (sta_netif) init_sta(sta_netif, sta_cfgs);
  return ESP_OK;
}

} // namespace drv::wifi
