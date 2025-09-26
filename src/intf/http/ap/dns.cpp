// Copyright (C) 2025 Vincent Hamp
// Copyright (C) 2025 Franziska Walter
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

/// DNS server for access point
///
/// \file   intf/http/ap/dns.cpp
/// \author Franziska Walter
/// \date   11/07/2025

#include "dns.hpp"
#include "log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

namespace intf::http::ap {

namespace {
#define DNS_PORT 53
#define DNS_MAX_PACKET_LEN 512

static bool get_ap_ip(uint8_t ip_out[4]); 

static int sock = -1;
static TaskHandle_t dns_task_handle = nullptr;
static void dns_task(void*) {
  LOGI("DNS: started");

  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(DNS_PORT);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0 ||
      bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    LOGE("DNS: socket error");
    return;
  }

  for (;;) {
    uint8_t buffer[DNS_MAX_PACKET_LEN] = {};
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    uint8_t ip[4];
    if (!get_ap_ip(ip)) {
        LOGE("DNS: ap ip could not be determined");
        continue; // oder: nutze Default-IP hier
    } else {
        // zur Kontrolle
        // LOGI("DNS: ap ip: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    }

    int len = recvfrom(sock,
                       buffer,
                       sizeof(buffer),
                       0,
                       (struct sockaddr*)&client_addr,
                       &addr_len);
    if (len < 12) continue;

    buffer[2] = 0x81;
    buffer[3] = 0x80;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x00;
    buffer[7] = 0x01;
    buffer[8] = 0x00;
    buffer[9] = 0x00;
    buffer[10] = 0x00;
    buffer[11] = 0x00;

    int ptr = len;
    buffer[ptr++] = 0xC0;
    buffer[ptr++] = 0x0C;
    buffer[ptr++] = 0x00;
    buffer[ptr++] = 0x01;
    buffer[ptr++] = 0x00;
    buffer[ptr++] = 0x01;
    buffer[ptr++] = 0x00;
    buffer[ptr++] = 0x00;
    buffer[ptr++] = 0x00;
    buffer[ptr++] = 0x3C;
    buffer[ptr++] = 0x00;
    buffer[ptr++] = 0x04;
    buffer[ptr++] = ip[0];
    buffer[ptr++] = ip[1];
    buffer[ptr++] = ip[2];
    buffer[ptr++] = ip[3];

    sendto(sock, buffer, ptr, 0, (struct sockaddr*)&client_addr, addr_len);
  }
}

static bool get_ap_ip(uint8_t ip_out[4]) {
  esp_netif_t* ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  if (!ap_netif) return false;

  esp_netif_ip_info_t ip_info;
  if (esp_netif_get_ip_info(ap_netif, &ip_info) != ESP_OK) return false;

  ip_out[0] = ip4_addr1(&ip_info.ip);
  ip_out[1] = ip4_addr2(&ip_info.ip);
  ip_out[2] = ip4_addr3(&ip_info.ip);
  ip_out[3] = ip4_addr4(&ip_info.ip);
  return true;
}
} // namespace

void start_dns() {
  if (dns_task_handle) {
    LOGI("DNS: resume server");
    vTaskResume(dns_task_handle);
  } else {
    LOGI("DNS: create server");
    xTaskCreate(dns_task, "dns_task", 4096, nullptr, 5, &dns_task_handle);
  }
}

void stop_dns() {
  if (dns_task_handle) {
    LOGI("DNS: suspend server");
    vTaskSuspend(dns_task_handle);
  }
}
} // namespace intf::http::ap
