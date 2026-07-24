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

#include "loco.hpp"
#include "log.h"

namespace mw::dcc {

/// \todo document
NvLocoBase::NvLocoBase(JsonVariantConst src) { fromJson(src); }

/// \todo document
void NvLocoBase::fromJson(JsonVariantConst src) {
  if (JsonVariantConst v{src["name"]}; v.is<std::string>())
    name = v.as<std::string>();

  if (JsonVariantConst v{src["mode"]}; v.is<Mode>())
    if (v.as<Mode>() != Mode::DCC) LOGE("Can't set mode to anything but DCC");

  if (JsonVariantConst v{src["speed_steps"]}; v.is<SpeedSteps>())
    speed_steps = v.as<SpeedSteps>();
}

/// \todo document
JsonDocument NvLocoBase::toJson() const {
  JsonDocument doc;
  doc["name"] = name;
  doc["mode"] = mode;
  doc["speed_steps"] = speed_steps;
  return doc;
}

/// \todo document
Loco::Loco(JsonVariantConst src) { fromJson(src); }

/// \todo document
void Loco::fromJson(JsonVariantConst src) {
  NvLocoBase::fromJson(src);

  if (JsonVariantConst v{src["rvvvvvvv"]}; v.is<uint8_t>()) rvvvvvvv = v;

  if (JsonVariantConst v{src["f31_0"]}; v.is<uint32_t>()) f31_0 = v;

  if (JsonObjectConst bidi_obj{src["bidi"].as<JsonObjectConst>()}) {
    if (JsonVariantConst v{bidi_obj["receive_counter"]}; v.is<uint32_t>())
      bidi.receive_counter = v.as<uint32_t>();

    if (JsonVariantConst v{bidi_obj["error_counter"]}; v.is<uint16_t>())
      bidi.error_counter = v.as<uint16_t>();

    if (JsonVariantConst v{bidi_obj["options"]}; v.is<uint8_t>())
      bidi.options = v.as<z21::RailComData::Options>();

    if (JsonVariantConst v{bidi_obj["speed"]}; v.is<uint8_t>())
      bidi.speed = v.as<uint8_t>();

    if (JsonVariantConst v{bidi_obj["qos"]}; v.is<uint8_t>())
      bidi.qos = v.as<uint8_t>();
  }
}

/// \todo document
JsonDocument Loco::toJson() const {
  auto doc{NvLocoBase::toJson()};
  doc["rvvvvvvv"] = rvvvvvvv;
  doc["f31_0"] = f31_0;

  JsonObject obj{doc["bidi"].to<JsonObject>()};
  obj["receive_counter"] = bidi.receive_counter;
  obj["error_counter"] = bidi.error_counter;
  obj["options"] = bidi.options;
  obj["speed"] = bidi.speed;
  obj["qos"] = bidi.qos;

  return doc;
}

} // namespace mw::dcc
