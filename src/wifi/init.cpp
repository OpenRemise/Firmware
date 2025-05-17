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

/// Initialize WiFi and mDNS
///
/// \file   wifi/init.cpp
/// \author Vincent Hamp
/// \date   02/07/2023

#pragma once

#include "init.hpp"
#include <driver/gpio.h>
#include <esp_mac.h>
#include <esp_wifi.h>
#include <bit>
#include <cstring>
#include <optional>
#include <vector>
#include <ztl/string.hpp>
#include "led/wifi.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"
#include "task_function.hpp"

namespace wifi {

namespace {

/// \todo document
wifi_ap_config_t ap_config() {
  constexpr auto ssid{"OpenRemise"};
  constexpr auto ssid_len{ztl::strlen(ssid)};
  wifi_ap_config_t ap{};
  memcpy(&ap.ssid, ssid, ssid_len);
  ap.ssid_len = ssid_len;
  ap.channel = 1u;
  ap.authmode = WIFI_AUTH_OPEN;
  ap.max_connection = 1u;
  ap.pmf_cfg = {
    .required = false,
  };
  return ap;
}

/// \todo document
std::optional<std::pair<wifi_sta_config_t, wifi_sta_config_t>>
optional_sta_configs() {
  mem::nvs::Settings nvs;
  std::pair<wifi_sta_config_t, wifi_sta_config_t> stas;

  // Read SSID
  if (auto const ssid{nvs.getStationSSID()}; size(ssid))
    std::ranges::copy(ssid, std::bit_cast<char*>(&stas.first.ssid));
  else {
    LOGI("sta_ssid setting doesn't exist");
    return std::nullopt;
  }

  // Read password
  if (auto const pass{nvs.getStationPassword()}; size(pass))
    std::ranges::copy(pass, std::bit_cast<char*>(&stas.first.password));

  // Read alternative SSID
  if (auto const ssid{nvs.getStationAlternativeSSID()}; size(ssid))
    std::ranges::copy(ssid, std::bit_cast<char*>(&stas.second.ssid));

  // Read alternative password
  if (auto const pass{nvs.getStationAlternativePassword()}; size(pass))
    std::ranges::copy(pass, std::bit_cast<char*>(&stas.second.password));

  // Allow connecting to open networks
  stas.first.threshold.authmode = WIFI_AUTH_OPEN;
  stas.second.threshold.authmode = WIFI_AUTH_OPEN;

  return stas;
}

/// \todo document
void event_handler(void*,
                   esp_event_base_t event_base,
                   int32_t event_id,
                   void* event_data) {
  // IP events
  if (event_base == IP_EVENT) {
    if (event_id == IP_EVENT_STA_GOT_IP) {
      // Set global IP string
      auto const event{std::bit_cast<ip_event_got_ip_t*>(event_data)};
      std::array<char, 16uz> ip;
      auto const count{
        snprintf(data(ip), size(ip), IPSTR, IP2STR(&event->ip_info.ip))};
      ip_str.replace(0uz, count, data(ip));
      LOGI("got ip: %s", ip_str.c_str());
      // Set global MAC string
      ESP_ERROR_CHECK(esp_base_mac_addr_get(data(mac)));
      snprintf(data(mac_str), size(mac_str), MACSTR, MAC2STR(mac));
      led::wifi(true);
    }
  }
  // WiFi events
  else if (event_base == WIFI_EVENT) {
    // STA connected to own AP
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
      auto const event{
        std::bit_cast<wifi_event_ap_staconnected_t*>(event_data)};
      LOGI("AP: STA " MACSTR " connected, AID=%d",
           MAC2STR(event->mac),
           event->aid);
      led::wifi(true);
    }
    // STA disconnected from own AP
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
      auto const event{
        std::bit_cast<wifi_event_ap_stadisconnected_t*>(event_data)};
      LOGI("AP: STA " MACSTR " disconnected, AID=%d",
           MAC2STR(event->mac),
           event->aid);
      led::wifi(false);
    }
    // STA disconnected from external AP
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
      auto const event{
        std::bit_cast<wifi_event_ap_stadisconnected_t*>(event_data)};
      LOGI("STA: " MACSTR " disconnected, AID=%d",
           MAC2STR(event->mac),
           event->aid);
      led::wifi(false);
      ip_str.clear();
      esp_wifi_connect();
    }
  }
}

/// \todo document
esp_err_t gpio_init() {
  static constexpr gpio_config_t io_conf{.pin_bit_mask = 1ull << boot_gpio_num,
                                         .mode = GPIO_MODE_INPUT,
                                         .pull_up_en = GPIO_PULLUP_ENABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE};
  return gpio_config(&io_conf);
}

/// \todo document
esp_err_t wifi_init() {
  // Common stuff
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  if (auto sta_netif{esp_netif_create_default_wifi_sta()})
    esp_netif_set_default_netif(sta_netif);
  else assert(false);
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_event_handler_register(
    IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
  return esp_wifi_start();
}

/// Scan access points and sort by RSSI
esp_err_t scan_ap_records() {
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
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
esp_err_t ap_init(wifi_ap_config_t const& ap_config) {
  if (auto ap_netif{esp_netif_create_default_wifi_ap()})
    esp_netif_set_default_netif(ap_netif);
  else assert(false);
  wifi_config_t wifi_config{.ap = ap_config};
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  return esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
}

/// \todo document
esp_err_t
sta_init(std::pair<wifi_sta_config_t, wifi_sta_config_t> const& sta_configs) {
  // Pick the configuration with the best RSSI
  wifi_config_t wifi_config{};
  auto first{cbegin(ap_records)};
  auto const last{cend(ap_records)};
  while (first < last) {
    if (!strcmp(std::bit_cast<char*>(&first->ssid),
                std::bit_cast<char*>(&sta_configs.first.ssid))) {
      wifi_config.sta = sta_configs.first;
      break;
    } else if (!strcmp(std::bit_cast<char*>(&first->ssid),
                       std::bit_cast<char*>(&sta_configs.second.ssid))) {
      wifi_config.sta = sta_configs.second;
      break;
    }
    ++first;
  }
  if (first == last) return ESP_FAIL;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  // Static IP
  mem::nvs::Settings nvs;
  auto const sta_ip{nvs.getStationIP()};
  auto const sta_netmask{nvs.getStationNetmask()};
  auto const sta_gateway{nvs.getStationGateway()};
  if (!sta_ip.empty() && !sta_netmask.empty() && !sta_gateway.empty()) {
    esp_netif_ip_info_t ip_info;
    ESP_ERROR_CHECK(esp_netif_str_to_ip4(sta_ip.c_str(), &ip_info.ip));
    ESP_ERROR_CHECK(
      esp_netif_str_to_ip4(sta_netmask.c_str(), &ip_info.netmask));
    ESP_ERROR_CHECK(esp_netif_str_to_ip4(sta_gateway.c_str(), &ip_info.gw));
    auto sta_netif{esp_netif_get_default_netif()};
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(sta_netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(sta_netif, &ip_info));
  }

  return esp_wifi_connect();
}

} // namespace

/// Initialize either
/// - STA (station) if NVS contains SSID/pass or
/// - AP (access point) if NVS doesn't contain SSID/pass or network not found
esp_err_t init(BaseType_t xCoreID) {
  ESP_ERROR_CHECK(gpio_init());
  ESP_ERROR_CHECK(wifi_init());
  ESP_ERROR_CHECK(scan_ap_records());

  // Try to connect to network
  if (auto const sta_configs{optional_sta_configs()};
      sta_configs && sta_init(*sta_configs) == ESP_OK) {
    if (!xTaskCreatePinnedToCore(task_function,
                                 task.name,
                                 task.stack_size,
                                 NULL,
                                 task.priority,
                                 &task.handle,
                                 xCoreID))
      assert(false);
    return ESP_OK;
  }
  // ... or fallback to AP
  else
    return ap_init(ap_config());
}

} // namespace wifi
