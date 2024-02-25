/// NVS "settings" namespace
///
/// \file   mem/nvs/settings.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "settings.hpp"
#include <dcc/dcc.hpp>

namespace mem::nvs {

/// TODO
std::string Settings::getStationmDNS() const { return getBlob("sta_mdns"); }

/// TODO
esp_err_t Settings::setStationmDNS(std::string_view str) {
  // TODO validate mdns
  return setBlob("sta_mdns", str);
}

/// TODO
std::string Settings::getStationSSID() const { return getBlob("sta_ssid"); }

/// TODO
esp_err_t Settings::setStationSSID(std::string_view str) {
  // TODO validate SSID
  return setBlob("sta_ssid", str);
}

/// TODO
std::string Settings::getStationPassword() const { return getBlob("sta_pass"); }

/// TODO
esp_err_t Settings::setStationPassword(std::string_view str) {
  // TODO validate password
  return setBlob("sta_pass", str);
}

/// TODO
uint16_t Settings::getHttpReceiveTimeout() const {
  return getU16("http_rx_timeout");
}

/// TODO
esp_err_t Settings::setHttpReceiveTimeout(uint16_t value) {
  if (value >= 5u && value <= 60u) return setU16("http_rx_timeout", value);
  else return ESP_ERR_INVALID_ARG;
}

/// TODO
uint16_t Settings::getHttpTransmitTimeout() const {
  return getU16("http_tx_timeout");
}

/// TODO
esp_err_t Settings::setHttpTransmitTimeout(uint16_t value) {
  if (value >= 5u && value <= 60u) return setU16("http_tx_timeout", value);
  else return ESP_ERR_INVALID_ARG;
}

/// TODO
uint8_t Settings::getDccPreamble() const { return getU8("dcc_preamble"); }

/// TODO
esp_err_t Settings::setDccPreamble(uint8_t value) {
  if (value >= DCC_TX_MIN_PREAMBLE_BITS && value <= 30u)
    return setU8("dcc_preamble", value);
  else return ESP_ERR_INVALID_ARG;
}

/// TODO
uint8_t Settings::getDccBit1Duration() const { return getU8("dcc_bit1_dur"); }

/// TODO
esp_err_t Settings::setDccBit1Duration(uint8_t value) {
  if (value >= dcc::tx::Bit1Min && value <= dcc::tx::Bit1Max)
    return setU8("dcc_bit1_dur", value);
  else return ESP_ERR_INVALID_ARG;
}

/// TODO
uint8_t Settings::getDccBit0Duration() const { return getU8("dcc_bit0_dur"); }

/// TODO
esp_err_t Settings::setDccBit0Duration(uint8_t value) {
  if (value >= dcc::tx::Bit0Min && value <= dcc::tx::Bit0Max)
    return setU8("dcc_bit0_dur", value);
  else return ESP_ERR_INVALID_ARG;
}

/// TODO
uint8_t Settings::getDccBiDiBitDuration() const {
  return getU8("dcc_bidibit_dur");
}

/// TODO
esp_err_t Settings::setDccBiDiBitDuration(uint8_t value) {
  return setU8("dcc_bidibit_dur", value);
}

}  // namespace mem::nvs