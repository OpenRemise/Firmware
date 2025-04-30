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

#pragma once

#include <ArduinoJson.h>
#include <optional>
#include <string>
#include <type_traits>
#include <z21/z21.hpp>
#include <ztl/string.hpp>

namespace dcc {

/// Non-volatile base
struct NvLocoBase : z21::LocoInfo {
  void fromJsonDocument(JsonDocument const& doc);
  JsonDocument toJsonDocument() const;
  std::string name{};
};

/// Actual object with volatile and non-volatile stuff
struct Loco : NvLocoBase {
  constexpr Loco() = default;
  Loco(JsonDocument const& doc);

  void fromJsonDocument(JsonDocument const& doc);
  JsonDocument toJsonDocument() const;

  static constexpr uint8_t min_priority{1u};
  static constexpr uint8_t max_priority{smath::pow(2, priority_bits) - 1};
  uint8_t priority{min_priority};

  struct BiDi {
    friend constexpr bool operator==(BiDi const&, BiDi const&) = default;

    z21::RailComData::Options options{};
    uint8_t speed{};
    uint8_t qos{};
  } bidi{};

  /// \todo remove
  void print() const {
    printf("Loco object:\n");
    printf("  name: %s\n", name.c_str());
    printf("  functions: " UINT32_TO_BINARY_FORMAT "\n",
           UINT32_TO_BINARY(f31_0));
    printf("  dir: %d\n", static_cast<bool>(rvvvvvvv & 0x80u));
    printf("  speed: %d\n", z21::decode_rvvvvvvv(speed_steps, rvvvvvvv));
  }
};

} // namespace dcc