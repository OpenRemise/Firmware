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

/// Analog documentation
///
/// \file   analog/doxygen.hpp
/// \author Vincent Hamp
/// \date   23/08/2024

#pragma once

namespace analog {

/// \page page_analog Analog
/// \tableofcontents
/// This module takes over all analog functions of the firmware. This includes
/// measuring track voltage and current, measuring temperature, and monitoring
/// all of those measurements.
///
/// \section section_analog_init Initialization
/// \copydetails init
///
/// \section section_analog_adc_task ADC task
/// \copydetails adc_task_function
///
/// \section section_analog_temp_task Temperature task
/// \copydetails temp_task_function
///
/// \section section_analog_conversions Conversions
/// The convert.hpp header provides some conversion functions to convert between
/// raw ADC values ​​and Si units. The following functions are defined:
/// - measurement2mV(VoltageMeasurement)
/// - mV2measurement(Voltage)
/// - measurement2mA(CurrentMeasurement)
/// - mA2measurement(Current)
///
/// <div class="section_buttons">
/// | Previous                | Next          |
/// | :---------------------- | ------------: |
/// | \ref page_api_reference | \ref page_dcc |
/// </div>

} // namespace analog