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

#include "loco.hpp"
#include "log.h"

namespace dcc {

///
void NvLocoBase::fromJsonDocument(JsonDocument const& doc) {
  // Copy name
  if (JsonVariantConst doc_name{doc["name"]}; doc_name.is<std::string>())
    name = doc_name.as<std::string>();

  // Copy name
  if (JsonVariantConst doc_speed_steps{doc["speed_steps"]};
      doc_speed_steps.is<z21::LocoInfo::SpeedSteps>())
    speed_steps = doc_speed_steps.as<z21::LocoInfo::SpeedSteps>();
}

///
JsonDocument NvLocoBase::toJsonDocument() const {
  JsonDocument doc;
  doc["name"] = name;
  doc["speed_steps"] = speed_steps;
  return doc;
}

///
void Loco::fromJsonDocument(JsonDocument const& doc) {
  NvLocoBase::fromJsonDocument(doc);

  // Copy rvvvvvvv
  if (JsonVariantConst doc_rvvvvvvv{doc["rvvvvvvv"]};
      doc_rvvvvvvv.is<uint8_t>())
    rvvvvvvv = doc_rvvvvvvv;

  // Copy f31_0
  if (JsonVariantConst doc_f31_0{doc["f31_0"]}; doc_f31_0.is<uint32_t>())
    f31_0 = doc_f31_0;
}

///
JsonDocument Loco::toJsonDocument() const {
  auto doc{NvLocoBase::toJsonDocument()};
  doc["rvvvvvvv"] = rvvvvvvv;
  doc["f31_0"] = f31_0;
  return doc;
}

} // namespace dcc