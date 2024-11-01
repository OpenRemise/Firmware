// Copyright (C) 2024 Vincent Hamp
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

///
///
/// \file   analog/convert.cpp
/// \author Vincent Hamp
/// \date   03/05/2023

#include "convert.hpp"

namespace analog {

namespace {

/// \todo document
int raw2mV(int meas) {
  int retval;
  adc_cali_raw_to_voltage(cali_handle, meas, &retval);
  return retval;
}

}  // namespace

/// \todo document
Voltage measurement2mV(VoltageMeasurement meas) {
  return static_cast<Voltage>((raw2mV(meas) * (r1 + r2)) / r2);
}

/// \todo document
VoltageMeasurement mV2measurement(Voltage mV) {
  return static_cast<VoltageMeasurement>((mV * r2 * max_measurement) /
                                         ((r1 + r2) * vref));
}

/// \todo document
Current measurement2mA(CurrentMeasurement meas) {
  return static_cast<Current>((raw2mV(meas) * kimon) / rimon);
}

/// \todo document
CurrentMeasurement mA2measurement(Current mA) {
  return static_cast<CurrentMeasurement>((mA * rimon * max_measurement) /
                                         (kimon * vref));
}

}  // namespace analog