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
#include "settings.hpp"

namespace mem::nvs {

/// Initialize NVS
///
/// nvs::init() is responsible for initializing the NVS memory during the boot
/// phase. If the NVS partition is truncated for any reason, the entire memory
/// is erased and then reinitialized. The default settings will be restored in
/// this case.
esp_err_t init() {
  auto err{nvs_flash_init()};

  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    ESP_ERROR_CHECK(nvs_flash_erase());

    // Retry nvs_flash_init and restore defaults
    /// \todo replace some defaults (e.g. loco flags) with Z21 definitions
    if (err = nvs_flash_init(); err == ESP_OK) {
      mem::nvs::Settings nvs;
      nvs.setStationmDNS("remise");
      nvs.setStationSSID("");
      nvs.setStationPassword("");
      nvs.setHttpReceiveTimeout(5u);
      nvs.setHttpTransmitTimeout(5u);
      nvs.setCurrentLimit(out::track::CurrentLimit::_4100mA);
      nvs.setCurrentLimitService(out::track::CurrentLimit::_1300mA);
      nvs.setCurrentShortCircuitTime(100u);
      nvs.setDccPreamble(DCC_TX_MIN_PREAMBLE_BITS);
      nvs.setDccBit1Duration(dcc::tx::Timing::Bit1);
      nvs.setDccBit0Duration(dcc::tx::Timing::Bit0);
      nvs.setDccBit0Duration(60u);
      nvs.setDccProgrammingType(3u);
      nvs.setDccStartupResetPacketCount(25u);
      nvs.setDccContinueResetPacketCount(6u);
      nvs.setDccProgramPacketCount(7u);
      nvs.setDccBitVerifyTo1(true);
      nvs.setDccProgrammingAckCurrent(50u);
      nvs.setDccLocoFlags(0x80u | 0x40u | 0x20u | 0x02u);
      nvs.setDccAccessoryFlags(0x04u);
    }
  }

  return err;
}

} // namespace mem::nvs
