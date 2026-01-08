// Copyright (C) 2026 Vincent Hamp
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

///
///
/// \file   mw/display/service.hpp
/// \author Vincent Hamp
/// \date   08/01/2026

#pragma once

#include <functional>
#include <string>

namespace mw::display {

/// \todo document
class Service {
public:
  Service(std::function<std::string()> json_sys_get_request);

private:
  // This gets called by FreeRTOS
  [[noreturn]] void taskFunction(void*);

  std::function<std::string()> _json_sys_get_request;
};

} // namespace mw::display
