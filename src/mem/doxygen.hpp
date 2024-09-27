/// Memory documentation
///
/// \file   mem/doxygen.hpp
/// \author Vincent Hamp
/// \date   19/02/2023

#pragma once

namespace mem {

/// \page page_mem Memory
/// \tableofcontents
/// \todo document mem page
///
/// | Setting         | NVS key         | Type   | Min | Max | Default  |
/// | --------------- | --------------- | ------ | --- | --- | -------- |
/// | bla             | sta_mdns        | Binary | -   | -   | "remise" |
/// | bla             | sta_ssid        | Binary | -   | -   | ""       |
/// | bla             | sta_pass        | Binary | -   | -   | ""       |
/// | bla             | http_rx_timeout | u8     | 5   | 60  | 5        |
/// | bla             | http_tx_timeout | u8     | 5   | 60  | 5        |
/// | bla             | usb_rx_timeout  | u8     | 1   | 10  | 1        |
/// | bla             | current_limit   | u8     | 0   | 3   | 3        |
/// | bla             | current_sc_time | u8     | 20  | 240 | 20       |
/// | bla             | dcc_preamble    | u8     | 17  | 30  | 17       |
/// | bla             | dcc_bit1_dur    | u8     | 56  | 60  | 58       |
/// | bla             | dcc_bit0_dur    | u8     | 97  | 114 | 100      |
/// | bla             | dcc_bidibit_dur | u8     | 57  | 61  | 60       |
/// | bla             | dcc_prog_type   | u8     | 0   | 3   | 3        |
/// | bla             | dcc_strtp_rs_pc | u8     | 25  | 255 | 25       |
/// | bla             | dcc_cntn_rs_pc  | u8     | 3   | 64  | 6        |
/// | bla             | dcc_prog_pc     | u8     | 2   | 64  | 7        |
/// | bla             | dcc_verify_bit1 | u8     | 0   | 1   | 1        |
/// | bla             | dcc_ack_cur     | u8     | 5   | 255 | 50       |
/// | bla             | dcc_flags       | u8     | ?   | ?   | ?        |
/// | bla             | mdu_preamble    | u8     | 14  | 30  | 14       |
/// | bla             | mdu_ackreq      | u8     | 10  | 30  | 10       |

}  // namespace mem