// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// HTTP websocket message
///
/// \file   http/message.hpp
/// \author Vincent Hamp
/// \date   12/04/2023

#pragma once

#include <esp_http_server.h>
#include <vector>

namespace http {

struct Message {
  int sock_fd;
  httpd_ws_type_t type;
  std::vector<uint8_t> payload;
};

}  // namespace http