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
/// \file   drv/anlg/convert.hpp
/// \author Vincent Hamp
/// \date   03/05/2023

#pragma once

#include <esp_adc/adc_cali.h>

namespace drv::anlg {

VccVoltage measurement2mV(VccVoltageMeasurement meas);
VccVoltageMeasurement mV2measurement(VccVoltage mV);
SupplyVoltage measurement2mV(SupplyVoltageMeasurement meas);
SupplyVoltageMeasurement mV2measurement(SupplyVoltage mV);
Current measurement2mA(CurrentMeasurement meas);
CurrentMeasurement mA2measurement(Current mA);

inline adc_cali_handle_t cali_handle{};

} // namespace drv::anlg
