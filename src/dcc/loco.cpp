#include "loco.hpp"
#include "log.h"

namespace dcc {

///
void NvLocoBase::fromJsonDocument(JsonDocument const& doc) {
  // Copy name
  if (JsonVariantConst doc_name{doc["name"]}; doc_name.is<std::string>())
    name = doc_name.as<std::string>();
}

///
DynamicJsonDocument NvLocoBase::toJsonDocument() const {
  DynamicJsonDocument doc{1024uz};
  doc["name"] = name;
  return doc;
}

///
void Loco::fromJsonDocument(JsonDocument const& doc) {
  NvLocoBase::fromJsonDocument(doc);

  // Copy functions
  if (JsonVariantConst doc_functions{doc["functions"]};
      doc_functions.is<uint64_t>())
    functions = doc_functions;

  // Copy speed
  if (JsonVariantConst doc_speed{doc["speed"]}; doc_speed.is<uint8_t>())
    speed = doc_speed;

  // Copy direction
  if (JsonVariantConst doc_dir{doc["dir"]}; doc_dir.is<int8_t>()) dir = doc_dir;
}

///
DynamicJsonDocument Loco::toJsonDocument() const {
  auto doc{NvLocoBase::toJsonDocument()};
  doc["functions"] = functions;
  doc["speed"] = speed;
  doc["dir"] = dir;
  return doc;
}

}  // namespace dcc