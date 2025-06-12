#include "usb_test.hpp"
#include "freertos_helpers.hpp"

// Create stream buffers
UsbTest::UsbTest() {
  intf::usb::rx_stream_buffer.handle =
    xStreamBufferCreate(intf::usb::rx_stream_buffer.size, 1uz);
  intf::usb::tx_stream_buffer.handle =
    xStreamBufferCreate(intf::usb::tx_stream_buffer.size, 1uz);
}

// Delete stream buffers
UsbTest::~UsbTest() {
  stream_buffer_delete_clear_handle(intf::usb::rx_stream_buffer.handle);
  stream_buffer_delete_clear_handle(intf::usb::tx_stream_buffer.handle);
}
