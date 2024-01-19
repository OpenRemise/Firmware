///
///
/// \file   analog/convert.hpp
/// \author Vincent Hamp
/// \date   03/05/2023

#pragma once

#include <esp_adc/adc_cali.h>
#include "current_range.hpp"

namespace analog {

CurrentRange get_current_range();
void set_current_range(CurrentRange range);
Voltage measurement2mV(VoltageMeasurement meas);
VoltageMeasurement mV2measurement(Voltage mV);
Current measurement2mA(CurrentMeasurement meas);
CurrentMeasurement mA2measurement(Current mA);

inline adc_cali_handle_t cali_handle{};

}  // namespace analog