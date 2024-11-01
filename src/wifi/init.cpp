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
#include <mdns.h>
#include <bit>
#include <cstring>
#include <optional>
#include <vector>
#include <ztl/string.hpp>
#include "log.h"
#include "mem/nvs/settings.hpp"

namespace wifi {

namespace {

/// \todo document
void led(uint32_t level) {
  ESP_ERROR_CHECK(gpio_set_level(led_gpio_num, level));
}

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
std::optional<wifi_sta_config_t> optional_sta_config() {
  mem::nvs::Settings nvs;
  wifi_sta_config_t sta{};

  // Read SSID
  if (auto const sta_ssid_str{nvs.getStationSSID()}; size(sta_ssid_str))
    std::ranges::copy(sta_ssid_str, std::bit_cast<char*>(&sta.ssid));
  else {
    LOGI("sta_ssid setting doesn't exist");
    return std::nullopt;
  }

  // Read password
  if (auto const sta_pass_str{nvs.getStationPassword()}; size(sta_pass_str)) {
    std::ranges::copy(sta_pass_str, std::bit_cast<char*>(&sta.password));
  } else {
    LOGI("sta_pass setting doesn't exist");
    return std::nullopt;
  }

  // Allow connecting to open networks
  sta.threshold.authmode = WIFI_AUTH_OPEN;

  return sta;
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
      led(true);
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
      led(true);
    }
    // STA disconnected from own AP
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
      auto const event{
        std::bit_cast<wifi_event_ap_stadisconnected_t*>(event_data)};
      LOGI("AP: STA " MACSTR " disconnected, AID=%d",
           MAC2STR(event->mac),
           event->aid);
      led(false);
    }
    // STA disconnected from external AP
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
      auto const event{
        std::bit_cast<wifi_event_ap_stadisconnected_t*>(event_data)};
      LOGI("STA: " MACSTR " disconnected, AID=%d",
           MAC2STR(event->mac),
           event->aid);
      led(false);
      ip_str.clear();
      esp_wifi_connect();
    }
  }
}

/// \todo document
esp_err_t gpio_init() {
  // Pulling this GPIO low can force AP init
  gpio_config_t io_conf{.pin_bit_mask = 1ull << force_ap_init_gpio_num,
                        .mode = GPIO_MODE_INPUT,
                        .pull_up_en = GPIO_PULLUP_ENABLE,
                        .pull_down_en = GPIO_PULLDOWN_DISABLE,
                        .intr_type = GPIO_INTR_DISABLE};
  ESP_ERROR_CHECK(gpio_config(&io_conf));

  // LED
  io_conf.pin_bit_mask = 1ull << led_gpio_num;
  io_conf.mode = GPIO_MODE_OUTPUT;
  return gpio_config(&io_conf);
}

/// \todo document
esp_err_t wifi_init() {
  // Common stuff
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  if (!esp_netif_create_default_wifi_sta()) assert(false);
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

/// \todo document
esp_err_t ap_init(wifi_ap_config_t const& ap_config) {
  LOGI("ap init");

  if (!esp_netif_create_default_wifi_ap()) assert(false);

  // Temporarily set mode to STA because AP doesn't support scanning
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  esp_wifi_scan_start(NULL, true);
  uint16_t ap_count{};
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
  std::vector<wifi_ap_record_t> ap_records(ap_count);
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, data(ap_records)));
  ap_records_queue.handle = xQueueCreate(ap_count, sizeof(wifi_ap_record_t));
  std::ranges::for_each(ap_records, [](auto&& ap_record) {
    if (!xQueueSend(ap_records_queue.handle, &ap_record, portMAX_DELAY))
      assert(false);
  });

  // Switch to access point
  wifi_config_t wifi_config{.ap = ap_config};
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  return ESP_OK;
}

/// \todo document
esp_err_t sta_init(wifi_sta_config_t const& sta_config) {
  LOGI("sta init");
  wifi_config_t wifi_config{.sta = sta_config};
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_connect());
  return ESP_OK;
}

/// \todo document
esp_err_t mdns_init(wifi_mode_t mode) {
  ESP_ERROR_CHECK(::mdns_init());

  mem::nvs::Settings nvs;
  auto const sta_mdns_str{nvs.getStationmDNS()};

  // STA mode: hostname is user setting
  // AP mode:  hostname fixed to "remise"
  mdns_str =
    mode == WIFI_MODE_STA && !empty(sta_mdns_str) ? sta_mdns_str : "remise";
  ESP_ERROR_CHECK(mdns_hostname_set(mdns_str.c_str()));

  mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);

  return ESP_OK;
}

}  // namespace

/// Initialize either
/// - AP (access point) if NVS doesn't contain SSID/pass or GPIO2 is high
/// - STA (station) if NVS contains SSID/pass and GPIO is low
esp_err_t init() {
  ESP_ERROR_CHECK(gpio_init());
  ESP_ERROR_CHECK(wifi_init());

  // Initialize AP if there is no STA config or AP pin is low
  auto const sta_config{optional_sta_config()};
  auto const force_ap_init{!gpio_get_level(force_ap_init_gpio_num)};
  auto const mode{!sta_config || force_ap_init ? WIFI_MODE_AP : WIFI_MODE_STA};
  ESP_ERROR_CHECK(mode == WIFI_MODE_AP ? ap_init(ap_config())
                                       : sta_init(*sta_config));

  // mDNS
  return mdns_init(mode);
}

}  // namespace wifi