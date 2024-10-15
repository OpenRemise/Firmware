// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// NVS "settings" namespace
///
/// \file   mem/nvs/settings.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <string>
#include "base.hpp"

namespace mem::nvs {

class Settings : public Base {
public:
  explicit Settings() : Base{"settings", NVS_READWRITE} {}

  std::string getStationmDNS() const;
  esp_err_t setStationmDNS(std::string_view str);

  std::string getStationSSID() const;
  esp_err_t setStationSSID(std::string_view str);

  std::string getStationPassword() const;
  esp_err_t setStationPassword(std::string_view str);

  uint8_t getHttpReceiveTimeout() const;
  esp_err_t setHttpReceiveTimeout(uint8_t value);

  uint8_t getHttpTransmitTimeout() const;
  esp_err_t setHttpTransmitTimeout(uint8_t value);

  uint8_t getUsbReceiveTimeout() const;
  esp_err_t setUsbReceiveTimeout(uint8_t value);

  out::track::CurrentLimit getCurrentLimit() const;
  esp_err_t setCurrentLimit(out::track::CurrentLimit value);

  uint8_t getCurrentShortCircuitTime() const;
  esp_err_t setCurrentShortCircuitTime(uint8_t value);

  uint8_t getDccPreamble() const;
  esp_err_t setDccPreamble(uint8_t value);

  uint8_t getDccBit1Duration() const;
  esp_err_t setDccBit1Duration(uint8_t value);

  uint8_t getDccBit0Duration() const;
  esp_err_t setDccBit0Duration(uint8_t value);

  uint8_t getDccBiDiBitDuration() const;
  esp_err_t setDccBiDiBitDuration(uint8_t value);

  uint8_t getDccProgrammingType() const;
  esp_err_t setDccProgrammingType(uint8_t value);

  uint8_t getDccStartupResetPacketCount() const;
  esp_err_t setDccStartupResetPacketCount(uint8_t value);

  uint8_t getDccContinueResetPacketCount() const;
  esp_err_t setDccContinueResetPacketCount(uint8_t value);

  uint8_t getDccProgramPacketCount() const;
  esp_err_t setDccProgramPacketCount(uint8_t value);

  bool getDccBitVerifyTo1() const;
  esp_err_t setDccBitVerifyTo1(bool value);

  uint8_t getDccProgrammingAckCurrent() const;
  esp_err_t setDccProgrammingAckCurrent(uint8_t value);

  uint8_t getDccFlags() const;
  esp_err_t setDccFlags(uint8_t value);

  uint8_t getMduPreamble() const;
  esp_err_t setMduPreamble(uint8_t value);

  uint8_t getMduAckreq() const;
  esp_err_t setMduAckreq(uint8_t value);
};

}  // namespace mem::nvs