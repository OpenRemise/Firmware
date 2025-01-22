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

/// Convert between ADC measurements and SI units
///
/// \file   analog/convert.cpp
/// \author Vincent Hamp
/// \date   03/05/2023

#include "convert.hpp"

namespace analog {

namespace {

/// Convert ADC measurement to mV
///
/// Converts a raw ADC measurement to mV and applies a cure fitting calibration.
///
/// \param  meas  ADC measurement
/// \return mV
int raw2mV(int meas) {
  int retval;
  adc_cali_raw_to_voltage(cali_handle, meas, &retval);
  return retval;
}

} // namespace

/// Convert VoltageMeasurement to Voltage
///
/// \param  meas  Voltage measurement
/// \return Voltage
Voltage measurement2mV(VoltageMeasurement meas) {
  return static_cast<Voltage>(
    (raw2mV(meas) * (voltage_upper_r + voltage_lower_r)) / voltage_lower_r);
}

/// Convert Voltage to VoltageMeasurement
///
/// \param  mV  Voltage in [mV]
/// \return VoltageMeasurement
VoltageMeasurement mV2measurement(Voltage mV) {
  return static_cast<VoltageMeasurement>(
    (mV * voltage_lower_r * max_measurement) /
    ((voltage_upper_r + voltage_lower_r) * vref));
}

/// Convert CurrentMeasurement to Current
///
/// \param  meas  Current measurement
/// \return Current
Current measurement2mA(CurrentMeasurement meas) {
  return static_cast<Current>((raw2mV(meas) * current_k) / current_r);
}

/// Convert Current to CurrentMeasurement
///
/// \param  mA  Current in [mA]
/// \return CurrentMeasurement
CurrentMeasurement mA2measurement(Current mA) {
  return static_cast<CurrentMeasurement>((mA * current_r * max_measurement) /
                                         (current_k * vref));
}

} // namespace analog