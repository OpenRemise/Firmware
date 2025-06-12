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

/// ZIMO documentation
///
/// \file   mw/zimo/doxygen.hpp
/// \author Vincent Hamp
/// \date   12/06/2025

#pragma once

namespace mw::zimo {

/// \page page_mw_zimo ZIMO
/// \details \tableofcontents
///
/// \section section_mw_zimo_decup DECUP
/// [ZPP](https://github.com/ZIMO-Elektronik/ZPP) and
/// [ZSU](https://github.com/ZIMO-Elektronik/ZSU) updates (WebSocket service)
///
/// \section section_mw_zimo_mdu MDU
/// [ZPP](https://github.com/ZIMO-Elektronik/ZPP) and
/// [ZSU](https://github.com/ZIMO-Elektronik/ZSU) updates (WebSocket service)
///
/// \section section_mw_zimo_ulf ULF
/// This module contains implementations of some of ZIMO's
/// [ULF_COM](https://github.com/ZIMO-Elektronik/ULF_COM) protocols. These
/// serial protocols are mainly used to update decoder firmware or sound.
///
/// \subsection subsection_mw_zimo_ulf_protocols Protocols
/// Before we go deeper into the implementation, here is a graphical overview of
/// the different operating modes and their transitions from the official
/// documentation. The transitions themselves are handled by the \ref
/// section_usb_receive "USB receive task".
///
// clang-format off
/// \page page_mw_zimo ZIMO
/// \details \tableofcontents
/// \image html https://raw.githubusercontent.com/ZIMO-Elektronik/ULF_COM/master/data/images/operating_modes.png "ULF_COM modes" width=480px
// clang-format on
/// \page page_mw_zimo ZIMO
/// \details \tableofcontents
///
/// Details on the individual protocols can be found in the corresponding GitHub
/// repositories.
///
/// \note
/// The tasks are **mutually exclusive** and share a \ref stack.
///
/// \subsection subsection_mw_ulf_init Initialization
/// \copydetails ulf::init
///
/// \subsection subsection_mw_ulf_dcc_ein DCC_EIN
/// \copydetails ulf::dcc_ein::task_function
///
/// \subsection subsection_mw_ulf_decup_ein DECUP_EIN
/// \copydetails ulf::decup_ein::task_function
///
/// \subsection subsection_mw_ulf_susiv2 SUSIV2
/// \copydetails ulf::susiv2::task_function
///
/// \section section_mw_zimo_zusi ZUSI
/// [ZPP](https://github.com/ZIMO-Elektronik/ZPP) updates (WebSocket service)
///
/// <div class="section_buttons">
/// | Previous          | Next          |
/// | :---------------- | ------------: |
/// | \ref page_mw_roco | \ref page_drv |
/// </div>

} // namespace mw::zimo
