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

/// Initialize USB
///
/// \file   intf/usb/init.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "init.hpp"
#include <tinyusb.h>
#include <tusb_cdc_acm.h>
#include <array>
#include "log.h"
#include "rx_task_function.hpp"
#include "tx_task_function.hpp"

namespace intf::usb {

namespace {

/// CDC receive callback
///
/// The callback reads data from the CDC device and copies it to
/// \ref rx_stream_buffer.
void tinyusb_cdc_rx_callback(int, cdcacm_event_t*) {
  std::array<uint8_t, buffer_size> buf;
  size_t bytes_received;
  if (tinyusb_cdcacm_read(
        TINYUSB_CDC_ACM_0, data(buf), size(buf), &bytes_received) != ESP_OK)
    LOGE("USB CDC read error");
  else
    xStreamBufferSend(rx_stream_buffer.handle, data(buf), bytes_received, 0u);
}

/// CDC line state changed callback
///
/// The callback stores changes to the RTS line in the global \ref rts.
void tinyusb_cdc_line_state_changed_callback(int, cdcacm_event_t* event) {
  rts = event->line_state_changed_data.rts;
}

} // namespace

/// Initialize USB
///
/// Initialization takes place in init(). This function creates a CDC device on
/// the [TinyUSB](https://docs.tinyusb.org/en/latest) stack, a \ref
/// rx_stream_buffer "receive-" and \ref tx_stream_buffer "transmit" buffer as
/// well as \ref rx_task_function "receive-" and  \ref tx_task_function
/// "transmit" tasks.
esp_err_t init() {
  rx_stream_buffer.handle =
    xStreamBufferCreate(rx_stream_buffer.size, sizeof(uint8_t));
  tx_stream_buffer.handle =
    xStreamBufferCreate(tx_stream_buffer.size, sizeof(uint8_t));

  rx_task.create(rx_task_function);
  tx_task.create(tx_task_function);

  static constexpr tinyusb_config_t tusb_cfg{
    .device_descriptor = NULL,
    .string_descriptor = NULL,
    .external_phy = false,
    .configuration_descriptor = NULL,
    /// \bug Currently can cause issues on Windows 11
    /// https://github.com/espressif/esp-idf/issues/14638
    //.self_powered = true,
    //.vbus_monitor_io = vbus_gpio_num
  };
  ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

  static constexpr tinyusb_config_cdcacm_t acm_cfg{
    .usb_dev = TINYUSB_USBDEV_0,
    .cdc_port = TINYUSB_CDC_ACM_0,
    .rx_unread_buf_sz = buffer_size,
    .callback_rx = &tinyusb_cdc_rx_callback,
    .callback_line_state_changed = &tinyusb_cdc_line_state_changed_callback};
  ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

  return ESP_OK;
}

} // namespace intf::usb
