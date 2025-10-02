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

#pragma once

#include <ArduinoJson.h>
#include <dcc/dcc.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <z21/z21.hpp>
#include <ztl/string.hpp>

namespace mw::dcc {

using namespace ::dcc;
namespace z21 = ::z21;

/// Non-volatile base
struct NvTurnoutBase : z21::TurnoutInfo {
  constexpr NvTurnoutBase() = default;
  explicit NvTurnoutBase(JsonDocument const& doc);

  void fromJsonDocument(JsonDocument const& doc);
  JsonDocument toJsonDocument() const;

  std::string name{};
  enum Type : uint16_t {
    // Special
    Unknown,
    Hidden,
    Custom,

    // Track (move)
    TurnoutRight = 256u,
    TurnoutLeft,
    TurnoutY,
    Turnout3Way,

    // Signal (guide)
    Signal2Aspects = 512u,
    Signal3Aspects,
    Signal4Aspects,
    SignalBlocking,
    SignalSemaphore2Aspects,
    SignalSemaphore3Aspects,

    // Scenery (world)
    Light = 768u,
    CrossingGate,
    Relay,

    /// \todo Railway signals of actual countries?
    AT = 1024u,
  } type{};
  struct Group {
    std::vector<Address::value_type> addresses{};
    std::vector<std::vector<Position>> positions{};
  } group{};
};

/// Actual object with volatile and non-volatile stuff
struct Turnout : NvTurnoutBase {
  constexpr Turnout() = default;
  explicit Turnout(JsonDocument const& doc);

  void fromJsonDocument(JsonDocument const& doc);
  JsonDocument toJsonDocument() const;

  TickType_t timeout_tick{}; ///<
};

} // namespace mw::dcc
