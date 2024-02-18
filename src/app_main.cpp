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
#include "utility.hpp"
#include "wifi/init.hpp"
#include "zusi/init.hpp"

#include "log.h"

/// ESP-IDF application entry point
extern "C" void app_main() {
  // Don't change initialization order
  ESP_ERROR_CHECK(invoke_on_core(0, trace::init));
  ESP_ERROR_CHECK(invoke_on_core(0, mem::init));
  ESP_ERROR_CHECK(invoke_on_core(0, wifi::init));
  ESP_ERROR_CHECK(invoke_on_core(1, analog::init));
  ESP_ERROR_CHECK(invoke_on_core(1, out::init));
  ESP_ERROR_CHECK(invoke_on_core(1, usb::init));
  ESP_ERROR_CHECK(invoke_on_core(0, http::init));

  // Those rely on http::init
  ESP_ERROR_CHECK(invoke_on_core(0, sys::init));
  ESP_ERROR_CHECK(invoke_on_core(1, dcc::init));
  ESP_ERROR_CHECK(invoke_on_core(1, mdu::init));
  ESP_ERROR_CHECK(invoke_on_core(1, ota::init));
  ESP_ERROR_CHECK(invoke_on_core(0, settings::init));
  ESP_ERROR_CHECK(invoke_on_core(1, zusi::init));

  //
  auto suspended{Mode::Suspended};
  mode.compare_exchange_strong(suspended, Mode::DCCService);
  vTaskDelay(pdMS_TO_TICKS(1000u));
  LOGI_TASK_RESUME(out::track::dcc::task.handle);

  bool expect_ack{};

  for (;;) {
    // Send reset for 2000ms
    TickType_t const then{xTaskGetTickCount() + pdMS_TO_TICKS(2000u)};
    while (xTaskGetTickCount() < then) {
      auto const packet{dcc::make_reset_packet()};
      xMessageBufferSend(
        out::tx_message_buffer.back_handle, data(packet), size(packet), 0u);
      vTaskDelay(pdMS_TO_TICKS(100u));
    }

    // Send 5x CV verify
    printf("\n\nACK %d\n", expect_ack);
    for (auto i{0uz}; i < 5uz; ++i) {
      auto const packet{dcc::make_cv_access_long_verify_service_packet(
        0u, expect_ack ? 3u : 42u)};
      xMessageBufferSend(
        out::tx_message_buffer.back_handle, data(packet), size(packet), 0u);
    }

    // Wait...
    vTaskDelay(pdMS_TO_TICKS(300u));
    bool bit{};
    xMessageBufferReceive(
      out::rx_message_buffer.handle, &bit, sizeof(bit), pdMS_TO_TICKS(200u));

    // Toggle expectation
    expect_ack = !expect_ack;
  }
}
