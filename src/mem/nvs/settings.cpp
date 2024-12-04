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
/// \file   mem/nvs/settings.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "settings.hpp"
#include <dcc/dcc.hpp>

namespace mem::nvs {

namespace {

/// \todo document
constexpr auto round_to(std::integral auto n, std::integral auto to) {
  auto const a{(n / to) * to};
  auto const b{a + to};
  return (n - a >= b - n) ? b : a;
}

}  // namespace

/// \todo document
std::string Settings::getStationmDNS() const { return getBlob("sta_mdns"); }

/// \todo document
esp_err_t Settings::setStationmDNS(std::string_view str) {
  return str.ends_with("remise") ? setBlob("sta_mdns", str)
                                 : ESP_ERR_INVALID_ARG;
}

/// \todo document
std::string Settings::getStationSSID() const { return getBlob("sta_ssid"); }

/// \todo document
esp_err_t Settings::setStationSSID(std::string_view str) {
  return setBlob("sta_ssid", str);
}

/// \todo document
std::string Settings::getStationPassword() const { return getBlob("sta_pass"); }

/// \todo document
esp_err_t Settings::setStationPassword(std::string_view str) {
  return setBlob("sta_pass", str);
}

/// \todo document
uint8_t Settings::getHttpReceiveTimeout() const {
  return getU8("http_rx_timeout");
}

/// \todo document
esp_err_t Settings::setHttpReceiveTimeout(uint8_t value) {
  return value >= 5u && value <= 60u ? setU8("http_rx_timeout", value)
                                     : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getHttpTransmitTimeout() const {
  return getU8("http_tx_timeout");
}

/// \todo document
esp_err_t Settings::setHttpTransmitTimeout(uint8_t value) {
  return value >= 5u && value <= 60u ? setU8("http_tx_timeout", value)
                                     : ESP_ERR_INVALID_ARG;
}

/// \todo document
out::track::CurrentLimit Settings::getCurrentLimit() const {
  return static_cast<out::track::CurrentLimit>(getU8("current_limit"));
}

/// \todo document
esp_err_t Settings::setCurrentLimit(out::track::CurrentLimit value) {
  return std::to_underlying(value) <=
             std::to_underlying(out::track::CurrentLimit::_4100mA)
           ? setU8("current_limit", std::to_underlying(value))
           : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getCurrentShortCircuitTime() const {
  return getU8("current_sc_time");
}

/// \todo document
esp_err_t Settings::setCurrentShortCircuitTime(uint8_t value) {
  return value >= 20u
           ? setU8("current_sc_time",
                   round_to(value,
                            (analog::conversion_frame_samples * 1000u) /
                              analog::sample_freq_hz))
           : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccPreamble() const { return getU8("dcc_preamble"); }

/// \todo document
esp_err_t Settings::setDccPreamble(uint8_t value) {
  return value >= DCC_TX_MIN_PREAMBLE_BITS && value <= DCC_TX_MAX_PREAMBLE_BITS
           ? setU8("dcc_preamble", value)
           : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccBit1Duration() const { return getU8("dcc_bit1_dur"); }

/// \todo document
esp_err_t Settings::setDccBit1Duration(uint8_t value) {
  return value >= DCC_TX_MIN_BIT_1_TIMING && value <= DCC_TX_MAX_BIT_1_TIMING
           ? setU8("dcc_bit1_dur", value)
           : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccBit0Duration() const { return getU8("dcc_bit0_dur"); }

/// \todo document
esp_err_t Settings::setDccBit0Duration(uint8_t value) {
  return value >= DCC_TX_MIN_BIT_0_TIMING && value <= DCC_TX_MAX_BIT_0_TIMING
           ? setU8("dcc_bit0_dur", value)
           : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccBiDiBitDuration() const {
  return getU8("dcc_bidibit_dur");
}

/// \todo document
esp_err_t Settings::setDccBiDiBitDuration(uint8_t value) {
  return !value || (value >= 57u && value <= 61u)
           ? setU8("dcc_bidibit_dur", value)
           : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccProgrammingType() const {
  return getU8("dcc_prog_type");
}

/// \todo document
esp_err_t Settings::setDccProgrammingType(uint8_t value) {
  return value <= 0x03u ? setU8("dcc_prog_type", value) : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccStartupResetPacketCount() const {
  return getU8("dcc_strtp_rs_pc");
}

/// \todo document
esp_err_t Settings::setDccStartupResetPacketCount(uint8_t value) {
  return value >= 25u ? setU8("dcc_strtp_rs_pc", value) : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccContinueResetPacketCount() const {
  return getU8("dcc_cntn_rs_pc");
}

/// \todo document
esp_err_t Settings::setDccContinueResetPacketCount(uint8_t value) {
  return value >= 3u && value <= 64u ? setU8("dcc_cntn_rs_pc", value)
                                     : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccProgramPacketCount() const {
  return getU8("dcc_prog_pc");
}

/// \todo document
esp_err_t Settings::setDccProgramPacketCount(uint8_t value) {
  return value >= 2u && value <= 64u ? setU8("dcc_prog_pc", value)
                                     : ESP_ERR_INVALID_ARG;
}

/// \todo document
bool Settings::getDccBitVerifyTo1() const {
  return static_cast<bool>(getU8("dcc_verify_bit1"));
}

/// \todo document
esp_err_t Settings::setDccBitVerifyTo1(bool value) {
  return setU8("dcc_verify_bit1", value);
}

/// \todo document
uint8_t Settings::getDccProgrammingAckCurrent() const {
  return getU8("dcc_ack_cur");
}

/// \todo document
esp_err_t Settings::setDccProgrammingAckCurrent(uint8_t value) {
  return value >= 5u ? setU8("dcc_ack_cur", value) : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getDccFlags() const { return getU8("dcc_flags"); }

/// \todo document
/// \todo replace the bit masks with definitions from the Z21 lib
esp_err_t Settings::setDccFlags(uint8_t value) {
  return setU8("dcc_flags", (value & ~0x03u) | 0x02u);  // Force DCC
}

/// \todo document
uint8_t Settings::getMduPreamble() const { return getU8("mdu_preamble"); }

/// \todo document
esp_err_t Settings::setMduPreamble(uint8_t value) {
  return value >= MDU_TX_MIN_PREAMBLE_BITS && value <= MDU_TX_MAX_PREAMBLE_BITS
           ? setU8("mdu_preamble", value)
           : ESP_ERR_INVALID_ARG;
}

/// \todo document
uint8_t Settings::getMduAckreq() const { return getU8("mdu_ackreq"); }

/// \todo document
esp_err_t Settings::setMduAckreq(uint8_t value) {
  return value >= MDU_TX_MIN_ACKREQ_BITS && value <= MDU_TX_MAX_ACKREQ_BITS
           ? setU8("mdu_ackreq", value)
           : ESP_ERR_INVALID_ARG;
}

}  // namespace mem::nvs