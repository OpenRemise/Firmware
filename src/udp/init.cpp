// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Initialize UDP
///
/// \file   udp/init.cpp
/// \author Vincent Hamp
/// \date   04/04/2024

#include "init.hpp"
#include <lwip/sockets.h>
#include "log.h"

namespace udp {

///
esp_err_t init() {
  //
  sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  assert(sock_fd >= 0);

  //
  sockaddr_in6 dest_addr;
  sockaddr_in* dest_addr_ip4{std::bit_cast<sockaddr_in*>(&dest_addr)};
  dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
  dest_addr_ip4->sin_family = AF_INET;
  dest_addr_ip4->sin_port = htons(port);
  auto const err{
    bind(sock_fd, std::bit_cast<sockaddr*>(&dest_addr), sizeof(dest_addr))};
  assert(err >= 0);

  return ESP_OK;
}

}  // namespace udp
