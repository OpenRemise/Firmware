/// Initialize USB
///
/// \file   usb/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <tinyusb.h>
#include <tusb_cdc_acm.h>
#include <array>
#include "dcc_ein/rx_task_function.hpp"
#include "dcc_ein/tx_task_function.hpp"
#include "log.h"
#include "rx_task_function.hpp"
#include "susiv2/rx_task_function.hpp"
#include "susiv2/tx_task_function.hpp"
#include "tx_task_function.hpp"

namespace usb {

namespace {

/// CDC receive callback
///
/// The callback reads data from the CDC device and copies it to \ref
/// rx_stream_buffer_handle.
void tinyusb_cdc_rx_callback(int, cdcacm_event_t*) {
  std::array<uint8_t, buffer_size> buf;
  size_t bytes_received;
  if (tinyusb_cdcacm_read(
        TINYUSB_CDC_ACM_0, data(buf), size(buf), &bytes_received) != ESP_OK)
    LOGE("USB CDC read error");
  else
    xStreamBufferSend(rx_stream_buffer.handle, data(buf), bytes_received, 0u);
}

}  // namespace

/// Initialize USB
///
/// Create a CDC device on the TinyUSB stack and start various USB and USB
/// protocol tasks. The following protocols are supported:
/// - DCC_EIN
/// - DECUP_EIN
/// - MDUSNDPREP
/// - SUSIV2
///
/// \return ESP_OK  No error
esp_err_t init() {
  rx_stream_buffer.handle = xStreamBufferCreate(rx_stream_buffer.size, 1uz);
  tx_stream_buffer.handle = xStreamBufferCreate(tx_stream_buffer.size, 1uz);
  xTaskCreatePinnedToCore(rx_task_function,
                          rx_task.name,
                          rx_task.stack_depth,
                          NULL,
                          rx_task.priority,
                          &rx_task.handle,
                          1);
  xTaskCreatePinnedToCore(tx_task_function,
                          tx_task.name,
                          tx_task.stack_depth,
                          NULL,
                          tx_task.priority,
                          &tx_task.handle,
                          1);

  xTaskCreatePinnedToCore(dcc_ein::rx_task_function,
                          dcc_ein::rx_task.name,
                          dcc_ein::rx_task.stack_depth,
                          NULL,
                          dcc_ein::rx_task.priority,
                          &dcc_ein::rx_task.handle,
                          1);
  xTaskCreatePinnedToCore(susiv2::rx_task_function,
                          susiv2::rx_task.name,
                          susiv2::rx_task.stack_depth,
                          NULL,
                          susiv2::rx_task.priority,
                          &susiv2::rx_task.handle,
                          1);

  // Transmit tasks must have lower priority than usb::tx_task
  xTaskCreatePinnedToCore(dcc_ein::tx_task_function,
                          dcc_ein::tx_task.name,
                          dcc_ein::tx_task.stack_depth,
                          NULL,
                          dcc_ein::tx_task.priority,
                          &dcc_ein::tx_task.handle,
                          1);
  xTaskCreatePinnedToCore(susiv2::tx_task_function,
                          susiv2::tx_task.name,
                          susiv2::tx_task.stack_depth,
                          NULL,
                          susiv2::tx_task.priority,
                          &susiv2::tx_task.handle,
                          1);

  static constexpr tinyusb_config_t tusb_cfg{.device_descriptor = NULL,
                                             .string_descriptor = NULL,
                                             .external_phy = false,
                                             .configuration_descriptor = NULL,
                                             .self_powered = true,
                                             .vbus_monitor_io = vbus_gpio_num};
  ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

  static constexpr tinyusb_config_cdcacm_t acm_cfg{
    .usb_dev = TINYUSB_USBDEV_0,
    .cdc_port = TINYUSB_CDC_ACM_0,
    .rx_unread_buf_sz = buffer_size,
    .callback_rx = &tinyusb_cdc_rx_callback};
  ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

  return ESP_OK;
}

}  // namespace usb
