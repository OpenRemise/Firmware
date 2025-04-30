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

/// NVS "settings" namespace
///
/// \file   mem/nvs/settings.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "settings.hpp"
#include <dcc/dcc.hpp>

namespace mem::nvs {

namespace {

/// Round integer towards
///
/// \param  n   Value to round
/// \param  to  Value to round towards to
/// \return Rounded value
constexpr auto round_to(std::integral auto n, std::integral auto to) {
  auto const a{(n / to) * to};
  auto const b{a + to};
  return (n - a >= b - n) ? b : a;
}

} // namespace

/// Get station mDNS
///
/// \return Station mDNS
std::string Settings::getStationmDNS() const { return getBlob("sta_mdns"); }

/// Set station mDNS
///
/// \param  str                           Station mDNS
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
/// \retval ESP_ERR_INVALID_ARG           Invalid mDNS
esp_err_t Settings::setStationmDNS(std::string_view str) {
  return str.ends_with("remise") ? setBlob("sta_mdns", str)
                                 : ESP_ERR_INVALID_ARG;
}

/// Get station SSID
///
/// \return Station SSID
std::string Settings::getStationSSID() const { return getBlob("sta_ssid"); }

/// Set station SSID
///
/// \param  str                           Station SSID
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Settings::setStationSSID(std::string_view str) {
  return setBlob("sta_ssid", str);
}

/// Get station password
///
/// \return Station password
std::string Settings::getStationPassword() const { return getBlob("sta_pass"); }

/// Set station password
///
/// \param  str                           Station password
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Settings::setStationPassword(std::string_view str) {
  return setBlob("sta_pass", str);
}

/// Get alternative station SSID
///
/// \return Alternative station SSID
std::string Settings::getAlternativeStationSSID() const {
  return getBlob("sta_alt_ssid");
}

/// Set alternative station SSID
///
/// \param  str                           Alternative station SSID
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Settings::setAlternativeStationSSID(std::string_view str) {
  return setBlob("sta_alt_ssid", str);
}

/// Get alternative station password
///
/// \return Alternative station password
std::string Settings::getAlternativeStationPassword() const {
  return getBlob("sta_alt_pass");
}

/// Set alternative station password
///
/// \param  str                           Alternative station password
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Settings::setAlternativeStationPassword(std::string_view str) {
  return setBlob("sta_alt_pass", str);
}

/// Get HTTP receive timeout
///
/// \return HTTP receive timeout [s]
uint8_t Settings::getHttpReceiveTimeout() const {
  return getU8("http_rx_timeout");
}

/// Set HTTP receive timeout
///
/// \param  value                         HTTP receive timeout [s]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           HTTP receive timeout out of range
esp_err_t Settings::setHttpReceiveTimeout(uint8_t value) {
  return value >= 5u && value <= 60u ? setU8("http_rx_timeout", value)
                                     : ESP_ERR_INVALID_ARG;
}

/// Get HTTP transmit timeout
///
/// \return HTTP transmit timeout [s]
uint8_t Settings::getHttpTransmitTimeout() const {
  return getU8("http_tx_timeout");
}

/// Set HTTP transmit timeout
///
/// \param  value                         HTTP transmit timeout [s]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           HTTP transmit timeout out of range
esp_err_t Settings::setHttpTransmitTimeout(uint8_t value) {
  return value >= 5u && value <= 60u ? setU8("http_tx_timeout", value)
                                     : ESP_ERR_INVALID_ARG;
}

/// Get current limit
///
/// \return Current limit
out::track::CurrentLimit Settings::getCurrentLimit() const {
  return static_cast<out::track::CurrentLimit>(getU8("cur_lim"));
}

/// Set current limit
///
/// \param  value                         Current limit
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           Current limit out of range
esp_err_t Settings::setCurrentLimit(out::track::CurrentLimit value) {
  return std::to_underlying(value) <=
             std::to_underlying(out::track::CurrentLimit::_4100mA)
           ? setU8("cur_lim", std::to_underlying(value))
           : ESP_ERR_INVALID_ARG;
}

/// Get current limit in service mode
///
/// \return Current limit in service mode
out::track::CurrentLimit Settings::getCurrentLimitService() const {
  return static_cast<out::track::CurrentLimit>(getU8("cur_lim_serv"));
}

/// Set current limit in service mode
///
/// \param  value                         Current limit in service mode
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           Current limit in service mode out of
///                                       range
esp_err_t Settings::setCurrentLimitService(out::track::CurrentLimit value) {
  return std::to_underlying(value) <=
             std::to_underlying(out::track::CurrentLimit::_4100mA)
           ? setU8("cur_lim_serv", std::to_underlying(value))
           : ESP_ERR_INVALID_ARG;
}

/// Get current limit in update mode
///
/// \return Current limit in update mode
out::track::CurrentLimit Settings::getCurrentLimitUpdate() const {
  return static_cast<out::track::CurrentLimit>(getU8("cur_lim_updt"));
}

/// Set current limit in update modes
///
/// \param  value                         Current limit in update mode
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           Current limit in update mode out of
///                                       range
esp_err_t Settings::setCurrentLimitUpdate(out::track::CurrentLimit value) {
  return std::to_underlying(value) <=
             std::to_underlying(out::track::CurrentLimit::_4100mA)
           ? setU8("cur_lim_updt", std::to_underlying(value))
           : ESP_ERR_INVALID_ARG;
}

/// Get LED duty cycle for bug LED
///
/// \return LED duty cycle for bug LED [%]
uint8_t Settings::getLedDutyCycleBug() const { return getU8("led_dc_bug"); }

/// Set LED duty cycle for bug LED
///
/// \param  value                         LED duty cycle [%]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           LED duty cycle out of range
esp_err_t Settings::setLedDutyCycleBug(uint8_t value) {
  return value <= 100u ? setU8("led_dc_bug", value) : ESP_ERR_INVALID_ARG;
}

/// Get LED duty cycle for WiFi LED
///
/// \return LED duty cycle for WiFi LED [%]
uint8_t Settings::getLedDutyCycleWiFi() const { return getU8("led_dc_wifi"); }

/// Set LED duty cycle for WiFi LED
///
/// \param  value                         LED duty cycle [%]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           LED duty cycle out of range
esp_err_t Settings::setLedDutyCycleWiFi(uint8_t value) {
  return value <= 100u ? setU8("led_dc_wifi", value) : ESP_ERR_INVALID_ARG;
}

/// Get current short circuit time
///
/// \return Current short circuit time [ms]
uint8_t Settings::getCurrentShortCircuitTime() const {
  return getU8("cur_sc_time");
}

/// Set current short circuit time
///
/// \param  value                         Current short circuit time [ms]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           Current short circuit time out of
///                                       range
esp_err_t Settings::setCurrentShortCircuitTime(uint8_t value) {
  return value >= 20u ? setU8("cur_sc_time",
                              round_to(value, analog::conversion_frame_time))
                      : ESP_ERR_INVALID_ARG;
}

/// Get DCC preamble count
///
/// \return DCC preamble count
uint8_t Settings::getDccPreamble() const { return getU8("dcc_preamble"); }

/// Set DCC preamble count
///
/// \param  value                         DCC preamble count
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC preamble count out of range
esp_err_t Settings::setDccPreamble(uint8_t value) {
  return value >= DCC_TX_MIN_PREAMBLE_BITS && value <= DCC_TX_MAX_PREAMBLE_BITS
           ? setU8("dcc_preamble", value)
           : ESP_ERR_INVALID_ARG;
}

/// Get DCC bit 1 duration
///
/// \return DCC bit 1 duration [µs]
uint8_t Settings::getDccBit1Duration() const { return getU8("dcc_bit1_dur"); }

/// Set DCC bit 1 duration
///
/// \param  value                         DCC bit 1 duration [µs]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC bit 1 duration out of range
esp_err_t Settings::setDccBit1Duration(uint8_t value) {
  return value >= DCC_TX_MIN_BIT_1_TIMING && value <= DCC_TX_MAX_BIT_1_TIMING
           ? setU8("dcc_bit1_dur", value)
           : ESP_ERR_INVALID_ARG;
}

/// Get DCC bit 0 duration
///
/// \return DCC bit 0 duration [µs]
uint8_t Settings::getDccBit0Duration() const { return getU8("dcc_bit0_dur"); }

/// Set DCC bit 0 duration
///
/// \param  value                         DCC bit 0 duration [µs]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC bit 0 duration out of range
esp_err_t Settings::setDccBit0Duration(uint8_t value) {
  return value >= DCC_TX_MIN_BIT_0_TIMING && value <= DCC_TX_MAX_BIT_0_TIMING
           ? setU8("dcc_bit0_dur", value)
           : ESP_ERR_INVALID_ARG;
}

/// Get DCC BiDi bit duration
///
/// \return DCC BiDi bit duration [µs]
uint8_t Settings::getDccBiDiBitDuration() const {
  return getU8("dcc_bidibit_dur");
}

/// Set DCC BiDi bit duration
///
/// \param  value                         DCC BiDi bit duration [µs]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC BiDi bit duration out of range
esp_err_t Settings::setDccBiDiBitDuration(uint8_t value) {
  return !value || (value >= 57u && value <= 61u)
           ? setU8("dcc_bidibit_dur", value)
           : ESP_ERR_INVALID_ARG;
}

/// Get DCC programming type
///
/// \return DCC programming type
uint8_t Settings::getDccProgrammingType() const {
  return getU8("dcc_prog_type");
}

/// Set DCC programming type
///
/// \param  value                         DCC programming type
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC programming type out of range
esp_err_t Settings::setDccProgrammingType(uint8_t value) {
  return value <= 0x03u ? setU8("dcc_prog_type", value) : ESP_ERR_INVALID_ARG;
}

/// Get DCC startup reset packet count
///
/// \return DCC startup reset packet count
uint8_t Settings::getDccStartupResetPacketCount() const {
  return getU8("dcc_strtp_rs_pc");
}

/// Set DCC startup reset packet count
///
/// \param  value                         DCC startup reset packet count
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC startup reset packet count out of
///                                       range
esp_err_t Settings::setDccStartupResetPacketCount(uint8_t value) {
  return value >= 25u ? setU8("dcc_strtp_rs_pc", value) : ESP_ERR_INVALID_ARG;
}

/// Get DCC continue reset packet count
///
/// \return DCC continue reset packet count
uint8_t Settings::getDccContinueResetPacketCount() const {
  return getU8("dcc_cntn_rs_pc");
}

/// Set DCC continue reset packet count
///
/// \param  value                         DCC continue reset packet count
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC continue reset packet count out of
///                                       range
esp_err_t Settings::setDccContinueResetPacketCount(uint8_t value) {
  return value >= 3u && value <= 64u ? setU8("dcc_cntn_rs_pc", value)
                                     : ESP_ERR_INVALID_ARG;
}

/// Get DCC program packet count
///
/// \return DCC program packet count
uint8_t Settings::getDccProgramPacketCount() const {
  return getU8("dcc_prog_pc");
}

/// Set DCC program packet count
///
/// \param  value                         DCC program packet count
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC program packet count out of range
esp_err_t Settings::setDccProgramPacketCount(uint8_t value) {
  return value >= 2u && value <= 64u ? setU8("dcc_prog_pc", value)
                                     : ESP_ERR_INVALID_ARG;
}

/// Get DCC bit verify
///
/// \return DCC bit verify
bool Settings::getDccBitVerifyTo1() const {
  return static_cast<bool>(getU8("dcc_verify_bit1"));
}

/// Set DCC bit verify
///
/// \param  value                         DCC bit verify
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
esp_err_t Settings::setDccBitVerifyTo1(bool value) {
  return setU8("dcc_verify_bit1", value);
}

/// Get DCC programming ack current
///
/// \return DCC programming ack current [mA]
uint8_t Settings::getDccProgrammingAckCurrent() const {
  return getU8("dcc_ack_cur");
}

/// Set DCC programming ack current
///
/// \param  value                         DCC programming ack current [mA]
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_INVALID_ARG           DCC programming ack current out of
///                                       range
esp_err_t Settings::setDccProgrammingAckCurrent(uint8_t value) {
  return value >= 10u && value <= 250u ? setU8("dcc_ack_cur", value)
                                       : ESP_ERR_INVALID_ARG;
}

/// Get DCC loco flags
///
/// \return DCC loco flags
uint8_t Settings::getDccLocoFlags() const { return getU8("dcc_loco_flags"); }

/// Set DCC loco flags
///
/// \param  value                         DCC loco flags
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
esp_err_t Settings::setDccLocoFlags(uint8_t value) {
  return setU8("dcc_loco_flags",
               (value & ~(z21::MmDccSettings::Flags::DccOnly |
                          z21::MmDccSettings::Flags::MmOnly)) |
                 z21::MmDccSettings::Flags::DccOnly);
}

/// Get DCC accessory flags
///
/// \return DCC accessory flags
uint8_t Settings::getDccAccessoryFlags() const {
  return getU8("dcc_accy_flags");
}

/// Set DCC accessory flags
///
/// \param  value                         DCC accessory flags
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
esp_err_t Settings::setDccAccessoryFlags(uint8_t value) {
  return setU8("dcc_accy_flags", value);
}

} // namespace mem::nvs