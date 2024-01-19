/// Documentation
///
/// \file   doxygen.hpp
/// \author Vincent Hamp
/// \date   26/02/2023

/// Welcome to the WULF developer documentation. Here's some lorem ipsum.
///
// clang-format off
/// \mainpage Introduction
/// | Chapter                                  | Content                                                             |
/// | ---------------------------------------- | ------------------------------------------------------------------- |
/// | \subpage page_architecture               | Architecture                                                        |
/// | \subpage page_program_structure_and_flow | Program structure and flow                                          |
// clang-format on
///
/// \page page_architecture Architecture
///
// clang-format off
/// \dot
/// digraph {
///   newrank=true;
///
///   level0->level1->level2;
///
///   subgraph cluster_dcc {
///     label="dcc";
///     executor;
///   }
///
///   subgraph cluster_ll {
///     label="ll";
///
///     subgraph cluster_ll_track {
///       label="track";
///
///       subgraph cluster_ll_track_dcc {
///         label="dcc";
///         ll_track_dcc
///       }
///
///       subgraph cluster_ll_track_mdu {
///         label="mdu";
///         ll_track_mdu
///       }
///     }
///
///     subgraph cluster_ll_zusi {
///       label="zusi";
///       ll_zusi
///     }
///   }
///
///   subgraph cluster_mem {
///     label="mem";
///
///     subgraph cluster_mem_nvs {
///       label="nvs";
///       nvs
///     }
///
///     subgraph cluster_mem_spiffs {
///       label="spiffs";
///       spiffs
///     }
///   }
///
///   subgraph cluster_http {
///     label="http";
///
///     subgraph cluster_http_ap {
///       label="ap";
///       ap_server
///     }
///
///     subgraph cluster_http_sta {
///       label="sta";
///       sta_server
///     }
///   }
///
///   subgraph cluster_usb {
///     label="usb";
///     usb_rx usb_tx
///
///     subgraph cluster_dcc_ein {
///       label="dcc_ein";
///       dcc_ein_rx dcc_ein_tx
///     }
///
///     subgraph cluster_susiv2 {
///       label="susiv2";
///       susiv2_rx susiv2_tx
///     }
///   }
///
///   ap_server->nvs[dir=both];
///
///   sta_server->nvs[dir=both];
///   sta_server->executor[label="DELETE"];
///   sta_server->executor[label="GET", dir=both];
///   sta_server->executor[label="PUT"];
///
///   executor->ll_track_dcc;
///   executor->nvs[dir=both];
///
///   usb_rx->dcc_ein_rx[label="Suspend/resume\nUSB RX", dir=both];
///   usb_rx->susiv2_rx[label="Suspend/resume\nUSB RX", dir=both];
///
///   dcc_ein_tx->usb_tx;
///   susiv2_tx->usb_tx;
///
///   dcc_ein_rx->ll_track_dcc;
///   ll_track_dcc->dcc_ein_tx;
///
///   susiv2_rx->ll_zusi;
///   ll_zusi->susiv2_tx;
///
///   spiffs->sta_server;
///
///   {rank=same; level0; usb_rx; usb_tx; ap_server; sta_server;}
///   {rank=same; level1; dcc_ein_rx; dcc_ein_tx; susiv2_rx; susiv2_tx; executor;}
///   {rank=same; level2; ll_track_mdu; ll_track_dcc; ll_zusi; spiffs; nvs}
/// }
/// \enddot
// clang-format on
/// \page page_architecture Architecture
///
/// \page page_program_structure_and_flow Program structure and flow
/// \tableofcontents
/// \section A
/// Some text in this section
///
/// \tableofcontents
/// \section B
/// More text in this one
///
///
/// \page page_modules Modules
/// Modules bla
///
// clang-format off
/// | Chapter            | Content                                         |
/// | ------------------ | ----------------------------------------------- |
/// | \subpage page_dcc  | DCC                                             |
/// | \subpage page_ll   | Low level                                       |
/// | \subpage page_mem  | SPIFFS and NVS memory                           |
/// | \subpage page_usb  | USB                                             |
/// | \subpage page_wifi | WiFi                                            |
/// | \subpage page_zusi | ZUSI                                            |
// clang-format on
/// \page page_modules Modules
///
