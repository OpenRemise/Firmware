///
///
/// \file   analog/convert.hpp
/// \author Vincent Hamp
/// \date   03/05/2023

#pragma once

#include <esp_adc/adc_cali.h>

namespace analog {

Voltage measurement2mV(VoltageMeasurement meas);
VoltageMeasurement mV2measurement(Voltage mV);
Current measurement2mA(CurrentMeasurement meas);
CurrentMeasurement mA2measurement(Current mA);

inline adc_cali_handle_t cali_handle{};

}  // namespace analog