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

/// Bug LED
///
/// \file   bug_led.cpp
/// \author Vincent Hamp
/// \date   25/01/2025

#include "bug_led.hpp"
#include <driver/gpio.h>

/// \todo document
void bug_led(uint32_t level) {
  ESP_ERROR_CHECK(gpio_set_level(bug_led_gpio_num, level));
}

/// \todo document
BugLed::BugLed(uint32_t level) { bug_led(level); }

/// \todo document
BugLed::~BugLed() { bug_led(false); }

/// \todo document
void BugLed::on() { bug_led(true); }

/// \todo document
void BugLed::off() { bug_led(false); }