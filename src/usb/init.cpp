// Copyright (C) 2024 Vincent Hamp
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

/// Initialize USB
///
/// \file   usb/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <tinyusb.h>
#include <tusb_cdc_acm.h>
#include <array>
#include "dcc_ein/task_function.hpp"
#include "decup_ein/task_function.hpp"
#include "log.h"
#include "rx_task_function.hpp"
#include "susiv2/task_function.hpp"
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
/// - MDU_EIN
/// - SUSIV2
///
/// \return ESP_OK  No error
esp_err_t init(BaseType_t xCoreID) {
  rx_stream_buffer.handle =
    xStreamBufferCreate(rx_stream_buffer.size, sizeof(uint8_t));
  tx_stream_buffer.handle =
    xStreamBufferCreate(tx_stream_buffer.size, sizeof(uint8_t));

  if (!xTaskCreatePinnedToCore(rx_task_function,
                               rx_task.name,
                               rx_task.stack_size,
                               NULL,
                               rx_task.priority,
                               &rx_task.handle,
                               xCoreID))
    assert(false);
  if (!xTaskCreatePinnedToCore(tx_task_function,
                               tx_task.name,
                               tx_task.stack_size,
                               NULL,
                               tx_task.priority,
                               &tx_task.handle,
                               xCoreID))
    assert(false);

  if (!xTaskCreatePinnedToCore(dcc_ein::task_function,
                               dcc_ein::task.name,
                               dcc_ein::task.stack_size,
                               NULL,
                               dcc_ein::task.priority,
                               &dcc_ein::task.handle,
                               xCoreID))
    assert(false);
  if (!xTaskCreatePinnedToCore(decup_ein::task_function,
                               decup_ein::task.name,
                               decup_ein::task.stack_size,
                               NULL,
                               decup_ein::task.priority,
                               &decup_ein::task.handle,
                               xCoreID))
    assert(false);
  if (!xTaskCreatePinnedToCore(susiv2::task_function,
                               susiv2::task.name,
                               susiv2::task.stack_size,
                               NULL,
                               susiv2::task.priority,
                               &susiv2::task.handle,
                               xCoreID))
    assert(false);

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
