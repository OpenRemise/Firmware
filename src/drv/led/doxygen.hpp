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

/// LED documentation
///
/// \file   drv/led/doxygen.hpp
/// \author Vincent Hamp
/// \date   27/04/2025

#pragma once

namespace drv::led {

/// \page page_led LED
/// \tableofcontents
/// This module handles dimming the LEDs. The brightness (duty cycle) can be
/// configured for each LED.
///
/// \section section_led_init Initialization
/// \copydetails init
///
/// \section section_led_bug Bug LED
/// The blue "bug LED" serves as an indicator for any kind of programming mode
/// (e.g. firmware update, DCC service mode, ...) as well as an error indicator.
/// The function \ref bug() can be used to switch the LED on or off with its set
/// duty cycle. The duty cycle can be read via
/// mem::nvs::Settings::getLedDutyCycleBug(). Since this LED is often used as an
/// update indicator, in addition to the free function there is also the RAII
/// wrapper Bug that can switch the LED on in its constructor and off in its
/// destructor.
///
/// \section section_led_wifi WiFi LED
/// The green "WiFi LED" indicates the network connection status. It lights up
/// when a connection is established and turns off when the connection is lost.
/// The function \ref wifi(bool) "wifi()" can be used to switch the LED on or
/// off with its set duty cycle. The duty cycle can be read via
/// mem::nvs::Settings::getLedDutyCycleWiFi().
///
/// <div class="section_buttons">
/// | Previous         | Next          |
/// | :--------------- | ------------: |
/// | \ref page_analog | \ref page_out |
/// </div>

} // namespace drv::led
