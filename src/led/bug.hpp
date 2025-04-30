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

/// Bug LED
///
/// \file   led/bug.hpp
/// \author Vincent Hamp
/// \date   27/04/2025

#pragma once

namespace led {

void bug(bool on);

struct Bug {
  Bug(bool);
  ~Bug();

  void on();
  void off();
};

} // namespace led