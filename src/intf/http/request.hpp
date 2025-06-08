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

/// HTTP request
///
/// \file   intf/http/request.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <string>

namespace intf::http {

struct Request {
  std::string uri; // endpoint + query
  std::string body;
};

} // namespace intf::http
