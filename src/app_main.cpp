/// ESP-IDF application entry point
///
/// \file   app_main.cpp
/// \author Vincent Hamp
/// \date   26/12/2022

#include "analog/init.hpp"
#include "dcc/init.hpp"
#include "http/init.hpp"
#include "mdu/init.hpp"
#include "mem/init.hpp"
#include "ota/init.hpp"
#include "out/init.hpp"
#include "settings/init.hpp"
#include "sys/init.hpp"
#include "trace.hpp"
#include "usb/init.hpp"
#include "wifi/init.hpp"
#include "zusi/init.hpp"

/// ESP-IDF application entry point
extern "C" void app_main() {
  // Don't change initialization order
  ESP_ERROR_CHECK(trace::init());
  ESP_ERROR_CHECK(mem::init());
  ESP_ERROR_CHECK(wifi::init());

  /*
    ESP_ERROR_CHECK(analog::init());
    ESP_ERROR_CHECK(out::init());
    ESP_ERROR_CHECK(usb::init());
    ESP_ERROR_CHECK(http::init());

    // Those rely on http::init
    ESP_ERROR_CHECK(sys::init());
    ESP_ERROR_CHECK(dcc::init());
    ESP_ERROR_CHECK(mdu::init());
    ESP_ERROR_CHECK(ota::init());
    ESP_ERROR_CHECK(settings::init());
    ESP_ERROR_CHECK(zusi::init());
  */

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(5000u));
    // esp_intr_dump(stdout);
  }
}
