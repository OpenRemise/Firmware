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

/// Initialize NVS
///
/// \file   mem/nvs/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <nvs_flash.h>
#include <z21/z21.hpp>
#include "settings.hpp"

namespace mem::nvs {

/// Initialize NVS
///
/// init() is responsible for initializing the NVS memory during the boot phase.
/// If the NVS partition is truncated for any reason, the entire memory is
/// erased and then reinitialized. The default settings will be restored in this
/// case.
esp_err_t init() {
  auto err{nvs_flash_init()};

  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    ESP_ERROR_CHECK(nvs_flash_erase());

    // Retry nvs_flash_init
    err = nvs_flash_init();
  }

  // Check if settings namespace contains keys, restore default values if not
  if (err == ESP_OK) {
    mem::nvs::Settings nvs;
    if (nvs.find("sta_mdns") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setStationmDNS("remise");
    if (nvs.find("sta_ssid") == ESP_ERR_NVS_NOT_FOUND) nvs.setStationSSID("");
    if (nvs.find("sta_pass") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setStationPassword("");
    if (nvs.find("sta_alt_ssid") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setStationAlternativeSSID("");
    if (nvs.find("sta_alt_pass") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setStationAlternativePassword("");
    if (nvs.find("sta_ip") == ESP_ERR_NVS_NOT_FOUND) nvs.setStationIP("");
    if (nvs.find("sta_netmask") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setStationNetmask("");
    if (nvs.find("sta_gateway") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setStationGateway("");
    if (nvs.find("http_rx_timeout") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setHttpReceiveTimeout(5u);
    if (nvs.find("http_tx_timeout") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setHttpTransmitTimeout(5u);
    if (nvs.find("http_exit_msg") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setHttpExitMessage(true);
    if (nvs.find("cur_lim") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setCurrentLimit(drv::out::track::CurrentLimit::_4100mA);
    if (nvs.find("cur_lim_serv") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setCurrentLimitService(drv::out::track::CurrentLimit::_1300mA);
    if (nvs.find("cur_sc_time") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setCurrentShortCircuitTime(100u);
    if (nvs.find("led_dc_bug") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setLedDutyCycleBug(5u);
    if (nvs.find("led_dc_wifi") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setLedDutyCycleWiFi(50u);
    if (nvs.find("dcc_preamble") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccPreamble(DCC_TX_MIN_PREAMBLE_BITS);
    if (nvs.find("dcc_bit1_dur") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccBit1Duration(dcc::tx::Timing::Bit1);
    if (nvs.find("dcc_bit0_dur") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccBit0Duration(dcc::tx::Timing::Bit0);
    if (nvs.find("dcc_bidibit_dur") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccBiDiBitDuration(60u);
    if (nvs.find("dcc_prog_type") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccProgrammingType(3u);
    if (nvs.find("dcc_strtp_rs_pc") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccStartupResetPacketCount(25u);
    if (nvs.find("dcc_cntn_rs_pc") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccContinueResetPacketCount(6u);
    if (nvs.find("dcc_prog_pc") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccProgramPacketCount(7u);
    if (nvs.find("dcc_verify_bit1") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccBitVerifyTo1(true);
    if (nvs.find("dcc_ack_cur") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccProgrammingAckCurrent(50u);
    if (nvs.find("dcc_loco_flags") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccLocoFlags(z21::MmDccSettings::Flags::DccShort127 |
                          z21::MmDccSettings::Flags::RepeatHfx |
                          z21::MmDccSettings::Flags::CV29AutomaticAddress |
                          z21::MmDccSettings::Flags::DccOnly);
    if (nvs.find("dcc_accy_flags") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccAccessoryFlags(0x04u);
    if (nvs.find("dcc_accy_swtime") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccAccessorySwitchTime(20u);
    if (nvs.find("dcc_accy_pc") == ESP_ERR_NVS_NOT_FOUND)
      nvs.setDccAccessorPacketCount(2u);
  }

  return err;
}

} // namespace mem::nvs
