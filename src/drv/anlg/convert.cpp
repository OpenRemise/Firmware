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
/// \file   drv/anlg/convert.cpp
/// \author Vincent Hamp
/// \date   03/05/2023

#include "convert.hpp"

namespace drv::anlg {

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

/// Convert measurement to voltage
///
/// \param  meas  Measurement
/// \return Voltage
int measurement2mV(int meas) {
  return (raw2mV(meas) * (voltage_upper_r + voltage_lower_r)) / voltage_lower_r;
}

/// Convert voltage to measurement
///
/// \param  mV  Voltage in [mV]
/// \return Voltage
int mV2measurement(int mV) {
  return (mV * voltage_lower_r * max_measurement) /
         ((voltage_upper_r + voltage_lower_r) * vref);
}

} // namespace

/// Convert VccVoltageMeasurement to VccVoltage
///
/// \param  meas  Vcc voltage measurement
/// \return VccVoltage
VccVoltage measurement2mV(VccVoltageMeasurement meas) {
  return static_cast<VccVoltage>(measurement2mV(static_cast<int>(meas)));
}

/// Convert VccVoltage to VccVoltageMeasurement
///
/// \param  mV  Vcc voltage in [mV]
/// \return VccVoltageMeasurement
VccVoltageMeasurement mV2measurement(VccVoltage mV) {
  return static_cast<VccVoltageMeasurement>(
    mV2measurement(static_cast<int>(mV)));
}

/// Convert SupplyVoltageMeasurement to SupplyVoltage
///
/// \param  meas  Supply voltage measurement
/// \return SupplyVoltage
SupplyVoltage measurement2mV(SupplyVoltageMeasurement meas) {
  return static_cast<SupplyVoltage>(measurement2mV(static_cast<int>(meas)));
}

/// Convert SupplyVoltage to SupplyVoltageMeasurement
///
/// \param  mV  Supply voltage in [mV]
/// \return SupplyVoltageMeasurement
SupplyVoltageMeasurement mV2measurement(SupplyVoltage mV) {
  return static_cast<SupplyVoltageMeasurement>(
    mV2measurement(static_cast<int>(mV)));
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

} // namespace drv::anlg
