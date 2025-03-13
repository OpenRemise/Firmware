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

namespace dcc {

/// \todo document
void NvLocoBase::fromJsonDocument(JsonDocument const& doc) {
  if (JsonVariantConst v{doc["name"]}; v.is<std::string>())
    name = v.as<std::string>();

  if (JsonVariantConst v{doc["mode"]}; v.is<z21::LocoInfo::Mode>())
    if (v.as<z21::LocoInfo::Mode>() != z21::LocoInfo::Mode::DCC)
      LOGE("Can't set mode to anything but DCC");

  if (JsonVariantConst v{doc["speed_steps"]}; v.is<z21::LocoInfo::SpeedSteps>())
    speed_steps = v.as<z21::LocoInfo::SpeedSteps>();
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
void Loco::fromJsonDocument(JsonDocument const& doc) {
  NvLocoBase::fromJsonDocument(doc);

  if (JsonVariantConst doc_rvvvvvvv{doc["rvvvvvvv"]};
      doc_rvvvvvvv.is<uint8_t>())
    rvvvvvvv = doc_rvvvvvvv;

  if (JsonVariantConst doc_f31_0{doc["f31_0"]}; doc_f31_0.is<uint32_t>())
    f31_0 = doc_f31_0;
}

/// \todo document
JsonDocument Loco::toJsonDocument() const {
  auto doc{NvLocoBase::toJsonDocument()};
  doc["rvvvvvvv"] = rvvvvvvv;
  doc["f31_0"] = f31_0;
  return doc;
}

} // namespace dcc