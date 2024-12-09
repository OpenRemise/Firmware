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

#pragma once

#include <esp_task.h>
#include <queue>
#include <ztl/inplace_vector.hpp>
#include <zusi/zusi.hpp>
#include "http/message.hpp"

namespace zusi {

class Service {
public:
  explicit Service(BaseType_t xCoreID);
  ~Service();

  esp_err_t socket(http::Message& msg);

private:
  // This gets called by FreeRTOS
  void taskFunction(void*);

  void loop();
  ztl::inplace_vector<uint8_t, 8uz - 1uz>
  transmit(std::vector<uint8_t> const& payload) const;
  void reset();

  ztl::inplace_vector<uint8_t, 8uz - 1uz> _data{};
  std::queue<http::Message> _queue{};
};

} // namespace zusi