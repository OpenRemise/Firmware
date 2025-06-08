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

///
///
/// \file   drv/out/track/current_limit.hpp
/// \author Vincent Hamp
/// \date   15/02/2024

#pragma once

#include <esp_err.h>

namespace drv::out::track {

CurrentLimit get_current_limit();
esp_err_t set_current_limit(CurrentLimit current_limit);

} // namespace drv::out::track
