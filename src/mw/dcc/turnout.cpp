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
void NvTurnoutBase::fromJsonDocument(JsonDocument const& doc) {}

/// \todo document
JsonDocument NvTurnoutBase::toJsonDocument() const {
  JsonDocument doc;
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
  auto doc{NvTurnoutBase::toJsonDocument()};
  return doc;
}

} // namespace mw::dcc
