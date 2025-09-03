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
NvLocoBase::NvLocoBase(JsonDocument const& doc) { fromJsonDocument(doc); }

/// \todo document
void NvLocoBase::fromJsonDocument(JsonDocument const& doc) {
  if (JsonVariantConst v{doc["name"]}; v.is<std::string>())
    name = v.as<std::string>();

  if (JsonVariantConst v{doc["mode"]}; v.is<Mode>())
    if (v.as<Mode>() != Mode::DCC) LOGE("Can't set mode to anything but DCC");

  if (JsonVariantConst v{doc["speed_steps"]}; v.is<SpeedSteps>())
    speed_steps = v.as<SpeedSteps>();
}

/// \todo document
JsonDocument NvLocoBase::toJsonDocument() const {
  JsonDocument doc;
  doc["name"] = name;
  doc["mode"] = mode;
  doc["speed_steps"] = speed_steps;
  return doc;
}

/// \todo document
Loco::Loco(JsonDocument const& doc) { fromJsonDocument(doc); }

/// \todo document
void Loco::fromJsonDocument(JsonDocument const& doc) {
  NvLocoBase::fromJsonDocument(doc);

  if (JsonVariantConst v{doc["rvvvvvvv"]}; v.is<uint8_t>()) rvvvvvvv = v;

  if (JsonVariantConst v{doc["f31_0"]}; v.is<uint32_t>()) f31_0 = v;

  if (JsonObjectConst obj{doc["bidi"].as<JsonObjectConst>()}) {
    if (JsonVariantConst v{obj["receive_counter"]}; v.is<uint32_t>())
      bidi.receive_counter = v.as<uint32_t>();

    if (JsonVariantConst v{obj["error_counter"]}; v.is<uint16_t>())
      bidi.error_counter = v.as<uint16_t>();

    if (JsonVariantConst v{obj["options"]}; v.is<uint8_t>())
      bidi.options = v.as<z21::RailComData::Options>();

    if (JsonVariantConst v{obj["speed"]}; v.is<uint8_t>())
      bidi.speed = v.as<uint8_t>();

    if (JsonVariantConst v{obj["qos"]}; v.is<uint8_t>())
      bidi.qos = v.as<uint8_t>();
  }
}

/// \todo document
JsonDocument Loco::toJsonDocument() const {
  auto doc{NvLocoBase::toJsonDocument()};
  doc["rvvvvvvv"] = rvvvvvvv;
  doc["f31_0"] = f31_0;

  JsonObject obj{doc.createNestedObject("bidi")};
  obj["receive_counter"] = bidi.receive_counter;
  obj["error_counter"] = bidi.error_counter;
  obj["options"] = bidi.options;
  obj["speed"] = bidi.speed;
  obj["qos"] = bidi.qos;

  return doc;
}

} // namespace mw::dcc
