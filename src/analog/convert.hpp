// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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