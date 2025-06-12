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

/// USB documentation
///
/// \file   intf/usb/doxygen.hpp
/// \author Vincent Hamp
/// \date   19/02/2023

#pragma once

namespace intf::usb {

/// \page page_usb USB
/// \tableofcontents
/// This module initializes the receive and transmit routines of the USB
/// interface. Currently only ZIMO protocols are supported via USB, but an
/// extension is conceivable.
///
/// \section section_usb_init Initialization
/// \copydetails init
///
/// \section section_usb_receive Receive task
/// \copydetails rx_task_function
///
/// \section section_usb_transmit Transmit task
/// \copydetails tx_task_function
///
/// <div class="section_buttons">
/// | Previous      | Next         |
/// | :------------ | -----------: |
/// | \ref page_udp | \ref page_mw |
/// </div>

} // namespace intf::usb
