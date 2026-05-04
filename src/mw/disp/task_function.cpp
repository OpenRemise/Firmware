// Copyright (C) 2026 Vincent Hamp
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

/// Display task function
///
/// \file   mw/disp/task_function.cpp
/// \author Vincent Hamp
/// \date   08/01/2026

#include "task_function.hpp"
#include <ArduinoJson.h>
#include <driver/uart.h>
#include <esp_app_desc.h>
#include <esp_task.h>
#include "drv/anlg/convert.hpp"
#include "mem/nvs/settings.hpp"

namespace mw::disp {

/// Display telemetry task
///
/// This task periodically emits a JSON status frame over `UART_NUM_0` when the
/// display extension is enabled.
///
/// The payload contains:
/// - firmware version
/// - IP
/// - \ref state "State"
/// - SSID
/// - mDNS
/// - RSSI
/// - Voltage
/// - Current
[[noreturn]] void task_function(void*) {
  using namespace drv::anlg;

  JsonDocument doc;
  auto const app_desc{esp_app_get_description()};
  doc["version"] = app_desc->version;

  static constexpr auto capacity{1024uz};
  std::string json{};
  json.reserve(capacity);

  for (;;) {
    if (mem::nvs::Settings nvs; nvs.getExtensionFlags() & 0b1u) {
      doc["ip"] = drv::wifi::ip_str;
      doc["state"] = magic_enum::enum_name(state.load());
      doc["ssid"] = nvs.getStationSSID();
      doc["mdns"] = intf::mdns::str;
      if (wifi_ap_record_t ap_record;
          esp_wifi_sta_get_ap_info(&ap_record) == ESP_OK)
        doc["rssi"] = ap_record.rssi;
      if (VccVoltageMeasurement meas;
          xQueuePeek(vcc_voltages_queue.handle, &meas, 0u))
        doc["vcc_voltage"] = measurement2mV(meas).value();
      if (SupplyVoltageMeasurement meas;
          xQueuePeek(supply_voltages_queue.handle, &meas, 0u))
        doc["supply_voltage"] = measurement2mV(meas).value();
      if (CurrentMeasurement meas;
          xQueuePeek(filtered_current_queue.handle, &meas, 0u))
        doc["current"] = measurement2mA(meas).value();

      //
      serializeJson(doc, json);
      assert(json.capacity() == capacity);
      uart_write_bytes(UART_NUM_0, data(json), size(json));
    }
    vTaskDelay(pdMS_TO_TICKS(task.timeout));
  }
}

} // namespace mw::disp
