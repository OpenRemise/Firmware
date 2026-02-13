// Copyright (C) 2026 Vincent Hamp
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

/// Initialize Ethernet
///
/// \file   drv/eth/init.cpp
/// \author Vincent Hamp
/// \date   28/01/2026

#pragma once

#include <driver/gpio.h>
#include <esp_eth_driver.h>
#include <esp_eth_netif_glue.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <cassert>
#include "drv/led/wifi.hpp"
#include "log.h"
#include "mem/nvs/settings.hpp"

namespace drv::eth {

namespace {

eth_link_t volatile link_status{ETH_LINK_DOWN};

/// \todo document
void event_handler(void*,
                   esp_event_base_t event_base,
                   int32_t event_id,
                   void* event_data) {
  // Ethernet got IP from connected AP
  if (event_base == IP_EVENT && event_id == IP_EVENT_ETH_GOT_IP) {
    auto const event{std::bit_cast<ip_event_got_ip_t*>(event_data)};
    auto const count{
      snprintf(data(ip), size(ip), IPSTR, IP2STR(&event->ip_info.ip))};
    ip_str.replace(0uz, count, data(ip));
    led::wifi(true);
    LOGI("IP_EVENT_ETH_GOT_IP %s", ip_str.c_str());
  }
  // Ethernet lost IP and the IP is reset to
  else if (event_base == IP_EVENT && event_id == IP_EVENT_ETH_LOST_IP) {
    ip.fill(0);
    ip_str.clear();
    led::wifi(false);
    LOGI("IP_EVENT_ETH_LOST_IP");
  }
  // Ethernet got a valid link
  else if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_CONNECTED) {
    link_status = ETH_LINK_UP;
    LOGI("ETHERNET_EVENT_CONNECTED");
  }
  // Ethernet lost a valid link
  else if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_DISCONNECTED) {
    link_status = ETH_LINK_DOWN;
    ip.fill(0);
    ip_str.clear();
    led::wifi(false);
    LOGI("ETHERNET_EVENT_DISCONNECTED");
  }
}

} // namespace

/// \todo document
esp_err_t init() {
  // Set global MAC string
  ESP_ERROR_CHECK(esp_base_mac_addr_get(data(mac)));
  snprintf(data(mac_str), size(mac_str), MACSTR, MAC2STR(mac));

  // Initialize TCP/IP stack and event loop
  esp_netif_init();
  esp_event_loop_create_default();

  static constexpr spi_bus_config_t buf_cfg{.mosi_io_num = mosi_gpio_num,
                                            .miso_io_num = miso_gpio_num,
                                            .sclk_io_num = sclk_gpio_num,
                                            .data2_io_num = -1,
                                            .data3_io_num = -1,
                                            .data4_io_num = -1,
                                            .data5_io_num = -1,
                                            .data6_io_num = -1,
                                            .data7_io_num = -1};

  spi_bus_initialize(SPI3_HOST, &buf_cfg, SPI_DMA_CH_AUTO);

  spi_device_interface_config_t dev_cfg{.mode = 0,
                                        .clock_speed_hz =
                                          static_cast<int>(30e6),
                                        .spics_io_num = cs_gpio_num,
                                        .queue_size = 20};

  eth_w5500_config_t w5500_cfg{.int_gpio_num = -1,
                               .poll_period_ms = 10u,
                               .spi_host_id = SPI3_HOST,
                               .spi_devcfg = &dev_cfg};

  // Init common MAC and PHY configs to default
  static constexpr eth_mac_config_t mac_cfg{.sw_reset_timeout_ms = 100u,
                                            .rx_task_stack_size = 4096u,
                                            .rx_task_prio = 15u,
                                            .flags = 0u};
  static constexpr eth_phy_config_t phy_cfg{.phy_addr = ESP_ETH_PHY_ADDR_AUTO,
                                            .reset_timeout_ms = 100u,
                                            .autonego_timeout_ms = 4000u,
                                            .reset_gpio_num = -1,
                                            .hw_reset_assert_time_us = 0,
                                            .post_hw_reset_delay_ms = 0};

  esp_eth_mac_t* eth_mac{esp_eth_mac_new_w5500(&w5500_cfg, &mac_cfg)};
  esp_eth_phy_t* eth_phy{esp_eth_phy_new_w5500(&phy_cfg)};

  // Init Ethernet driver to default and install it
  esp_eth_handle_t eth_handle{NULL};
  esp_eth_config_t eth_cfg_spi = ETH_DEFAULT_CONFIG(eth_mac, eth_phy);
  if (auto const err{esp_eth_driver_install(&eth_cfg_spi, &eth_handle)}) {
    LOGE("esp_eth_driver_install failed %s", esp_err_to_name(err));
    ESP_ERROR_CHECK(eth_phy->del(eth_phy));
    ESP_ERROR_CHECK(eth_mac->del(eth_mac));
    ESP_ERROR_CHECK(spi_bus_free(SPI3_HOST));
    return ESP_ERR_NOT_FOUND;
  }

  // W5500 has no internal unique MAC
  ESP_ERROR_CHECK(esp_base_mac_addr_get(data(mac)));
  ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, data(mac)));

  // Create the network interface handle
  esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
  esp_netif_t* netif{esp_netif_new(&netif_cfg)};
  assert(netif);

  // Attach
  esp_eth_netif_glue_handle_t eth_glue{esp_eth_new_netif_glue(eth_handle)};
  ESP_ERROR_CHECK(esp_netif_attach(netif, eth_glue));
  ESP_ERROR_CHECK(esp_netif_set_default_netif(netif));

  // Register event handlers
  ESP_ERROR_CHECK(esp_event_handler_register(
    IP_EVENT, IP_EVENT_ETH_GOT_IP, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    IP_EVENT, IP_EVENT_ETH_LOST_IP, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    ETH_EVENT, ETHERNET_EVENT_CONNECTED, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
    ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &event_handler, NULL));

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
    auto netif{esp_netif_get_default_netif()};
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));
  }

  ESP_ERROR_CHECK(esp_eth_start(eth_handle));

  // Wait for ETHERNET_EVENT_CONNECTED
  for (auto const then{xTaskGetTickCount() + pdMS_TO_TICKS(5'000u)};
       xTaskGetTickCount() < then;)
    if (link_status == ETH_LINK_UP) return ESP_OK;
    else vTaskDelay(pdMS_TO_TICKS(100u));

  return ESP_ERR_TIMEOUT;
}

} // namespace drv::eth
