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

/// ULF documentation
///
/// \file   mw/ulf/doxygen.hpp
/// \author Vincent Hamp
/// \date   04/05/2025

#pragma once

namespace mw::ulf {

/// \page page_ulf ULF
/// \tableofcontents
/// This module contains implementations of some of ZIMO's
/// [ULF_COM](https://github.com/ZIMO-Elektronik/ULF_COM) protocols. These
/// serial protocols are mainly used to update decoder firmware or sound.
///
/// \section section_protocols Protocols
/// Before we go deeper into the implementation, here is a graphical overview of
/// the different operating modes and their transitions from the official
/// documentation. The transitions themselves are handled by the \ref
/// section_usb_receive "USB receive task".
///
// clang-format off
/// \page page_ulf ULF
/// \details \tableofcontents
/// \image html https://raw.githubusercontent.com/ZIMO-Elektronik/ULF_COM/master/data/images/operating_modes.png "ULF_COM modes" width=480px
// clang-format on
/// \page page_ulf ULF
/// \details \tableofcontents
///
/// Details on the individual protocols can be found in the corresponding GitHub
/// repositories.
///
/// \note
/// The tasks are **mutually exclusive** and share a \ref stack.
///
/// \section section_ulf_init Initialization
/// \copydetails init
///
/// \subsection subsection_protocols_ulf_dcc_ein ULF_DCC_EIN
/// \copydetails dcc_ein::task_function
///
/// \subsection subsection_protocols_ulf_decup_ein ULF_DECUP_EIN
/// \copydetails decup_ein::task_function
///
/// \subsection subsection_protocols_ulf_susiv2 ULF_SUSIV2
/// \copydetails susiv2::task_function
///
/// <div class="section_buttons">
/// | Previous      | Next          |
/// | :------------ | ------------: |
/// | \ref page_udp | \ref page_usb |
/// </div>

} // namespace mw::ulf
