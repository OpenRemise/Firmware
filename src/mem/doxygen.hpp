// Copyright (C) 2024 Vincent Hamp
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

/// Memory documentation
///
/// \file   mem/doxygen.hpp
/// \author Vincent Hamp
/// \date   19/02/2023

#pragma once

namespace mem {

/// \page page_mem Memory
/// \details \tableofcontents
/// This module takes care of all used memory in the firmware. Currently, this
/// only includes the initialization and maintenance of the NVS storage.
///
/// \section section_init Initialization
/// \copydetails mem::init
///
/// \page page_mem Memory
/// \details \tableofcontents
/// \section section_mem_nvs NVS
/// Non-volatile storage (NVS) is designed to store key-value pairs in flash.
/// The firmware uses this memory type to store accessories, locomotives, and
/// settings.
///
/// \subsection subsection_nvs_init Initialization
/// \copydetails nvs::init
///
/// \subsection subsection_mem_nvs_base Base
/// \copydetails nvs::Base
///
/// \subsection subsection_mem_nvs_accessories Accessories
/// \copydetails nvs::Accessories
///
/// \subsection subsection_mem_nvs_locos Locos
/// \copydetails nvs::Locos
///
/// \subsection subsection_mem_nvs_settings Settings
/// \copydetails nvs::Settings
///
/// The following table contains an overview of all currently available
/// settings, their NVS key, value range and default.
///
// clang-format off
/// \page page_mem Memory
/// \details \tableofcontents
/// | Setting                                                                                                                                          | NVS Key         | Type   | Min | Max | Default  |
/// | ------------------------------------------------------------------------------------------------------------------------------------------------ | --------------- | ------ | --- | --- | -------- |
/// | [mDNS](https://en.wikipedia.org/wiki/Multicast_DNS) hostname under which the device appears (e.g. my-remise.local)                               | sta_mdns        | Binary | -   | -   | "remise" |
/// | Name of the network to connect to                                                                                                                | sta_ssid        | Binary | -   | -   | ""       |
/// | Password of the network to connect to                                                                                                            | sta_pass        | Binary | -   | -   | ""       |
/// | Timeout for receiving [HTTP](https://en.wikipedia.org/wiki/HTTP) requests (also used for USB)                                                    | http_rx_timeout | u8     | 5   | 60  | 5        |
/// | Timeout for transmitting HTTP response                                                                                                           | http_tx_timeout | u8     | 5   | 60  | 5        |
/// | Current limit in [DCC](https://github.com/ZIMO-Elektronik/DCC) operation mode                                                                    | cur_lim         | u8     | 0   | 3   | 3        |
/// | Current limit in DCC service mode                                                                                                                | cur_lim_serv    | u8     | 0   | 3   | 1        |
/// | Time after which an overcurrent is considered a short circuit                                                                                    | cur_sc_time     | u8     | 20  | 240 | 20       |
/// | Number of DCC preamble bits                                                                                                                      | dcc_preamble    | u8     | 17  | 30  | 17       |
/// | Duration of a DCC 1 bit                                                                                                                          | dcc_bit1_dur    | u8     | 56  | 60  | 58       |
/// | Duration of a DCC 0 bit                                                                                                                          | dcc_bit0_dur    | u8     | 97  | 114 | 100      |
/// | Duration of a DCC BiDi bit (during cutout)                                                                                                       | dcc_bidibit_dur | u8     | 57  | 61  | 60       |
/// | How a DCC service mode verify is performed                                                                                                       | dcc_prog_type   | u8     | 0   | 3   | 3        |
/// | Number of DCC reset packets at the start of the service mode programming sequence                                                                | dcc_strtp_rs_pc | u8     | 25  | 255 | 25       |
/// | Number of DCC reset packets when continuing the service mode programming sequence                                                                | dcc_cntn_rs_pc  | u8     | 3   | 64  | 6        |
/// | Number of DCC programming packets in the service mode programming sequence                                                                       | dcc_prog_pc     | u8     | 2   | 64  | 7        |
/// | Comparing bits to either 0 or 1 during a service mode verify                                                                                     | dcc_verify_bit1 | u8     | 0   | 1   | 1        |
/// | DCC acknowledge pulse current (60mA according to [S-9.2.3](https://www.nmra.org/sites/default/files/standards/sandrp/DCC/S/S-9.2.3_2012_07.pdf)) | dcc_ack_cur     | u8     | 10  | 250 | 50       |
/// | Number of [MDU](https://github.com/ZIMO-Elektronik/MDU) preamble bits                                                                            | mdu_preamble    | u8     | 14  | 30  | 14       |
/// | Number of MDU ackreq bits                                                                                                                        | mdu_ackreq      | u8     | 10  | 30  | 10       |
// clang-format on
/// \page page_mem Memory
/// \details \tableofcontents
///
/// <div class="section_buttons">
/// | Previous      | Next          |
/// | :------------ | ------------: |
/// | \ref page_mdu | \ref page_ota |
/// </div>

} // namespace mem