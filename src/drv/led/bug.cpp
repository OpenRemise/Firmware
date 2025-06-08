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
/// \file   drv/led/bug.cpp
/// \author Vincent Hamp
/// \date   27/04/2025

#include "bug.hpp"
#include <driver/ledc.h>
#include "mem/nvs/settings.hpp"

namespace drv::led {

/// Turn bug LED on or off
///
/// \param  on  LED state
void bug(bool on) {
  // Apply duty cycle
  if (on) {
    mem::nvs::Settings nvs;
    auto const dc{nvs.getLedDutyCycleBug()};
    ESP_ERROR_CHECK(
      ledc_set_duty(LEDC_LOW_SPEED_MODE, bug_channel, (dc * 256u) / 100u));
  }
  // ... don't care
  else
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, bug_channel, 0u));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, bug_channel));
}

/// But LED ctor
///
/// \param  on  LED state
Bug::Bug(bool on) { bug(on); }

/// Bug LED dtor
Bug::~Bug() { off(); }

/// Turn bug LED on
void Bug::on() { bug(true); }

/// Turn bug LED off
void Bug::off() { bug(false); }

} // namespace drv::led
