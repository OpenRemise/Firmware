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

#include "turnout.hpp"
#include "log.h"

namespace mw::dcc {

/// \todo document
NvTurnoutBase::NvTurnoutBase(JsonDocument const& doc) { fromJsonDocument(doc); }

/// \todo document
void NvTurnoutBase::fromJsonDocument(JsonDocument const& doc) {
  if (JsonVariantConst v{doc["name"]}; v.is<std::string>())
    name = v.as<std::string>();

  if (JsonVariantConst v{doc["mode"]}; v.is<Mode>())
    if (v.as<Mode>() != Mode::DCC) LOGE("Can't set mode to anything but DCC");

  if (JsonVariantConst v{doc["position"]}; v.is<uint8_t>()) position = v;

  if (JsonVariantConst v{doc["type"]}; v.is<Type>()) type = v.as<Type>();

  if (JsonObjectConst obj{doc["group"].as<JsonObjectConst>()}) {
    if (JsonVariantConst arr{obj["addresses"]}; arr.is<JsonArrayConst>()) {
      group.addresses.clear();
      group.addresses.reserve(std::size(arr));
      for (auto const v : arr.as<JsonArrayConst>())
        group.addresses.push_back(v.as<Address::value_type>());
    }

    if (JsonVariantConst outer{obj["positions"]}; outer.is<JsonArrayConst>()) {
      group.positions.clear();
      group.positions.reserve(std::size(outer));
      for (auto const inner : outer.as<JsonArrayConst>()) {
        std::vector<Position> positions;
        positions.reserve(std::size(inner));
        for (auto const v : inner.as<JsonArrayConst>())
          positions.push_back(v.as<Position>());
        group.positions.push_back(move(positions));
      }
    }
  }
}

/// \todo document
JsonDocument NvTurnoutBase::toJsonDocument() const {
  JsonDocument doc;
  doc["name"] = name;
  doc["mode"] = mode;
  doc["position"] = position;
  doc["type"] = type;

  JsonObject obj{doc.createNestedObject("group")};

  JsonArray arr{obj.createNestedArray("addresses")};
  for (auto const v : group.addresses) arr.add(v);

  JsonArray outer{obj.createNestedArray("positions")};
  for (auto const& positions : group.positions) {
    JsonArray inner{outer.createNestedArray()};
    for (auto const v : positions) inner.add(v);
  }

  return doc;
}

/// \todo document
Turnout::Turnout(JsonDocument const& doc) { fromJsonDocument(doc); }

/// \todo document
void Turnout::fromJsonDocument(JsonDocument const& doc) {
  NvTurnoutBase::fromJsonDocument(doc);
}

/// \todo document
JsonDocument Turnout::toJsonDocument() const {
  return NvTurnoutBase::toJsonDocument();
}

} // namespace mw::dcc
