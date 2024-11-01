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