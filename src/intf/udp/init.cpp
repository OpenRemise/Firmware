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

/// Initialize UDP
///
/// \file   intf/udp/init.cpp
/// \author Vincent Hamp
/// \date   04/04/2024

#include "init.hpp"
#include <lwip/sockets.h>
#include "log.h"

namespace intf::udp {

/// \todo document
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

} // namespace intf::udp
