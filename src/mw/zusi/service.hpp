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
#include <ulf/susiv2.hpp>
#include "intf/http/message.hpp"

namespace mw::zusi {

/// ZUSI service
///
/// Those are the service details
class Service {
public:
  Service();

  esp_err_t socket(intf::http::Message& msg);

private:
  [[noreturn]] void taskFunction(void*);
  void loop();
  ::ulf::susiv2::Response transmit(std::vector<uint8_t> const& payload) const;
  void close();

  ::ulf::susiv2::Response _resp{};
  std::queue<intf::http::Message> _queue{};
};

} // namespace mw::zusi
