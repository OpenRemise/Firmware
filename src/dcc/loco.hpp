#pragma once

#include <ArduinoJson.h>
#include <optional>
#include <string>
#include <type_traits>

namespace dcc {

/// Non-volatile base
struct NvLocoBase {
  void fromJsonDocument(JsonDocument const& doc);
  DynamicJsonDocument toJsonDocument() const;
  std::string name{};
};

// Standard layout allows slicing assignment
static_assert(std::is_standard_layout_v<NvLocoBase>);

/// Actual object with volatile and non-volatile stuff
struct Loco : NvLocoBase {
  void fromJsonDocument(JsonDocument const& doc);
  DynamicJsonDocument toJsonDocument() const;
  uint64_t functions{};
  uint8_t speed{};
  int8_t dir{};

  // TODO remove
  void print() const {
    printf("Loco object:\n");
    printf("  name:%s\n", data(name));
    printf("  functions:%X%X\n",
           static_cast<uint32_t>(functions >> 32u),
           static_cast<uint32_t>(functions));
    printf("  speed:%d\n", speed);
    printf("  dir:%d\n", dir);
  }
};

}  // namespace dcc