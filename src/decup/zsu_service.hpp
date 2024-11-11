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
#include <array>
#include <queue>
#include <ulf/decup_ein.hpp>
#include "http/message.hpp"

namespace decup {

class ZsuService : public ulf::decup_ein::rx::Base {
public:
  esp_err_t zsuSocket(http::Message& msg);

protected:
  void loop();

private:
  uint8_t transmit(std::span<uint8_t const> bytes) final;
  void done() final;

  void close();

  std::queue<http::Message> _queue{};
  std::optional<uint8_t> _ack{};
};

}  // namespace decup