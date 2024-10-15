// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Cover /ota/ endpoint
///
/// \file   ota/service.hpp
/// \author Vincent Hamp
/// \date   13/06/2023

#pragma once

#include <esp_err.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <queue>
#include "http/message.hpp"

namespace ota {

class Service {
public:
  explicit Service(BaseType_t xCoreID);
  ~Service();

  esp_err_t socket(http::Message& msg);

private:
  // This gets called by FreeRTOS
  void taskFunction(void*);

  void loop();
  uint8_t write(std::vector<uint8_t> const& payload);
  void end();
  void close();

  std::queue<http::Message> _queue{};
  esp_partition_t const* _partition{};
  esp_ota_handle_t _handle{};
  uint8_t _ack{};
};

}  // namespace ota