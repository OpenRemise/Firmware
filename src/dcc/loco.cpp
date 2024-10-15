// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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

}  // namespace dcc