///
///
/// \file   analog/convert.cpp
/// \author Vincent Hamp
/// \date   03/05/2023

#include "convert.hpp"

namespace analog {

namespace {

/// TODO
int raw2mV(int meas) {
  int retval;
  adc_cali_raw_to_voltage(cali_handle, meas, &retval);
  return retval;
}

}  // namespace

/// TODO
Voltage measurement2mV(VoltageMeasurement meas) {
  return static_cast<Voltage>((raw2mV(meas) * (r1 + r2)) / r2);
}

/// TODO
VoltageMeasurement mV2measurement(Voltage mV) {
  return static_cast<VoltageMeasurement>((mV * r2 * max_measurement) /
                                         ((r1 + r2) * vref));
}

/// TODO
Current measurement2mA(CurrentMeasurement meas) {
  return static_cast<Current>((raw2mV(meas) * kimon) / rimon);
}

/// TODO
CurrentMeasurement mA2measurement(Current mA) {
  return static_cast<CurrentMeasurement>((mA * rimon * max_measurement) /
                                         (kimon * vref));
}

}  // namespace analog