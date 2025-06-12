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

/// General utilities
///
/// \file   utility.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "utility.hpp"
#include <ArduinoJson.h>
#include <esp_system.h>
#include "log.h"
#include "mem/nvs/settings.hpp"

namespace {

/// Restart in 1s
[[noreturn]] void restart_in_1s(void*) {
  vTaskDelay(pdMS_TO_TICKS(1000u));
  esp_restart();
}

} // namespace

void esp_delayed_restart() {
  LOGI("esp_delayed_restart");
  xTaskCreate(restart_in_1s, NULL, 1024uz, NULL, tskIDLE_PRIORITY, NULL);
}

// https://arduinojson.org/v6/how-to/validate-json/
bool validate_json(std::string_view json) {
  JsonDocument doc, filter;
  return deserializeJson(doc, json, DeserializationOption::Filter(filter)) ==
         DeserializationError::Ok;
}

///
std::optional<dcc::Address::value_type> uri2address(std::string_view uri) {
  dcc::Address::value_type addr{};
  auto const pos{uri.find_last_of('/')};
  if (pos == std::string_view::npos) return std::nullopt;
  auto const last{cend(uri)};
  auto const [ptr, ec]{std::from_chars(&uri[pos] + 1, last, addr)};
  if (ec != std::errc{} || ptr != last) return std::nullopt;
  return addr;
}

///
std::optional<dcc::Address> uri2loco_address(std::string_view uri) {
  auto addr{uri2address(uri)};
  if (!addr) return std::nullopt;
  return dcc::Address{.value = *addr,
                      .type = *addr < 128u ? dcc::Address::BasicLoco
                                           : dcc::Address::ExtendedLoco};
}

/// \warning
/// Do not use this function in time-critical code. Always cache the value!
uint32_t http_receive_timeout2ms() {
  mem::nvs::Settings nvs;
  return nvs.getHttpReceiveTimeout() * 1000u;
}
