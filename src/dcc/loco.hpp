// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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
  void fromJsonDocument(JsonDocument const& doc);
  JsonDocument toJsonDocument() const;

  static constexpr uint8_t min_priority{1u};
  static constexpr uint8_t max_priority{smath::pow(2, priority_bits) - 1};
  uint8_t priority{min_priority};

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

}  // namespace dcc