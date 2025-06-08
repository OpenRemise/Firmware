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

/// Deinitialize peripherals when suspending DCC task
///
/// \file   drv/out/track/dcc/suspend.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <esp_err.h>

namespace drv::out::track::dcc {

esp_err_t deinit_gpio();
esp_err_t deinit_bidi();
esp_err_t deinit_alarm();
esp_err_t deinit_rmt();
esp_err_t deinit_encoder();
esp_err_t suspend();

} // namespace drv::out::track::dcc
