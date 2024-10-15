// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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