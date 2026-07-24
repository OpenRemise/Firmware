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

#include "accessory.hpp"
#include "log.h"

namespace mw::dcc {

/// \todo document
void NvAccessoryBase::fromJson([[maybe_unused]] JsonVariantConst src) {}

/// \todo document
JsonDocument NvAccessoryBase::toJson() const {
  JsonDocument doc;
  return doc;
}

/// \todo document
Accessory::Accessory(JsonVariantConst src) { fromJson(src); }

/// \todo document
void Accessory::fromJson(JsonVariantConst src) {
  NvAccessoryBase::fromJson(src);
}

/// \todo document
JsonDocument Accessory::toJson() const {
  auto doc{NvAccessoryBase::toJson()};
  return doc;
}

} // namespace mw::dcc
