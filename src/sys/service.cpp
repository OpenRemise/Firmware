/// Cover /sys/ endpoint
///
/// \file   sys/service.cpp
/// \author Vincent Hamp
/// \date   13/08/2023

#pragma once

#include "service.hpp"
#include <ArduinoJson.h>
#include <esp_app_desc.h>
#include <algorithm>
#include <magic_enum.hpp>
#include "analog/convert.hpp"
#include "log.h"
#include "utility.hpp"

namespace sys {

using namespace analog;

/// TODO
http::Response Service::getRequest(http::Request const& req) {
  //
  DynamicJsonDocument doc{1024uz};

  doc["mode"] = magic_enum::enum_name(mode.load());

  auto const app_desc{esp_app_get_description()};
  doc["version"] = app_desc->version;
  doc["idf_version"] = app_desc->idf_ver;
  doc["compile_date"] = app_desc->date;

  doc["ip"] = wifi::ip_str;
  doc["mac"] = wifi::mac_str;

  doc["heap"] = esp_get_free_heap_size();
  doc["internal_heap"] = esp_get_free_internal_heap_size();

  if (VoltagesQueue::value_type voltages;
      xQueuePeek(voltages_queue.handle, &voltages, 0u))
    doc["voltage"] =
      measurement2mV(static_cast<VoltageMeasurement>(
                       std::accumulate(cbegin(voltages), cend(voltages), 0) /
                       size(voltages)))
        .value();

  if (CurrentsQueue::value_type currents;
      xQueuePeek(currents_queue.handle, &currents, 0u))
    doc["current"] =
      measurement2mA(static_cast<CurrentMeasurement>(
                       std::accumulate(cbegin(currents), cend(currents), 0) /
                       size(currents)))
        .value();

  if (TemperatureQueue::value_type temp;
      xQueuePeek(temperature_queue.handle, &temp, 0u))
    doc["temperature"] = temp;

  //
  std::string json;
  json.reserve(1024uz);
  serializeJson(doc, json);

  return json;
}

/// TODO
http::Response Service::postRequest(http::Request const& req) {
  LOGI("%s", __PRETTY_FUNCTION__);
  LOGI("uri %s", req.uri.c_str());
  LOGI("body %s", req.body.c_str());

  // Validate body
  if (!validate_json(req.body))
    return std::unexpected<std::string>{"415 Unsupported Media Type"};

  // Deserialize (this only creates meta data which points to the source)
  DynamicJsonDocument doc{size(req.body) * 10u};
  if (auto const err{deserializeJson(doc, data(req.body), size(req.body))}) {
    LOGE("Deserialization failed %s", err.c_str());
    return std::unexpected<std::string>{"500 Internal Server Error"};
  }

  if (JsonVariantConst v{doc["mode"]}; v.is<std::string>()) {
    auto const str{v.as<std::string>()};

    if (auto const m{magic_enum::enum_cast<Mode>(str)}) switch (*m) {
        case Mode::Suspended: [[fallthrough]];
        case Mode::Blocked:
          if (auto op{Mode::DCCOperations}, serv{Mode::DCCService};
              mode.compare_exchange_strong(op, Mode::Blocked) ||
              mode.compare_exchange_strong(serv, Mode::Blocked))
            return {};
          else break;
        case Mode::DCCOperations: [[fallthrough]];
        case Mode::DCCService:
          if (auto suspended{Mode::Suspended};
              mode.compare_exchange_strong(suspended, *m)) {
            LOGI_TASK_RESUME(dcc::task.handle);
            LOGI_TASK_RESUME(out::track::dcc::task.handle);
            return {};
          } else break;
        default: break;
      }

    // Got mode, but couldn't handle it
    return std::unexpected<std::string>{"422 Unprocessable Entity"};
  }

  //
  return {};
}

}  // namespace sys