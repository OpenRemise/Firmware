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

/// Driver documentation
///
/// \file   drv/doxygen.hpp
/// \author Vincent Hamp
/// \date   10/06/2025

#pragma once

namespace drv {

/// \page page_drv Drivers
/// \details
/// The driver modules serve the different peripherals such as SPI, RMT or WiFi.
///
// clang-format off
/// \page page_drv Driver
/// \details
/// | Chapter                  | Namespace   | Content                                      |
/// | ------------------------ | ----------- | ---------------------------------------------|
/// | \subpage page_drv_analog | \ref analog | ADC measurements, overcurrent                |
/// | \subpage page_drv_led    | \ref led    | Dimming LEDs                                 |
/// | \subpage page_drv_out    | \ref out    | Generation of various SUSI and track signals |
/// | \subpage page_drv_trace  | \ref trace  | Debug purposes                               |
/// | \subpage page_drv_wifi   | \ref wifi   | Setup WiFi in AP or STA mode                 |
// clang-format on
/// \page page_drv Driver
/// \details
///
/// <div class="section_buttons">
/// | Previous          | Next                 |
/// | :---------------- | -------------------: |
/// | \ref page_mw_zimo | \ref page_drv_analog |
/// </div>

} // namespace drv
