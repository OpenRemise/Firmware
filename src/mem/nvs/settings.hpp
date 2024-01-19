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

  uint16_t getHttpReceiveTimeout() const;
  esp_err_t setHttpReceiveTimeout(uint16_t value);

  uint16_t getHttpTransmitTimeout() const;
  esp_err_t setHttpTransmitTimeout(uint16_t value);

  uint8_t getDccPreamble() const;
  esp_err_t setDccPreamble(uint8_t value);

  uint8_t getDcc1Duration() const;
  esp_err_t setDcc1Duration(uint8_t value);

  uint8_t getDcc0Duration() const;
  esp_err_t setDcc0Duration(uint8_t value);

  bool getDccBiDi() const;
  esp_err_t setDccBiDi(bool value);
};

}  // namespace mem::nvs