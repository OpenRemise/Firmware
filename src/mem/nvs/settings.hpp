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