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

#pragma once

#include <esp_task.h>
#include <queue>
#include <ulf/decup_ein.hpp>
#include "intf/http/message.hpp"

namespace decup {

class Service : public ulf::decup_ein::rx::Base {
public:
  Service();
  ~Service();

  esp_err_t zppSocket(intf::http::Message& msg);
  esp_err_t zsuSocket(intf::http::Message& msg);

private:
  esp_err_t socket(intf::http::Message& msg, State decup_state);
  void taskFunction(void*);
  void loop();

  uint8_t transmit(std::span<uint8_t const> bytes) final;

  void close();

  std::queue<intf::http::Message> _queue{};
  std::optional<uint8_t> _ack{};
};

} // namespace decup
