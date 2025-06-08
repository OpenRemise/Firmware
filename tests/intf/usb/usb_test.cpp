#include "usb_test.hpp"
#include "freertos_helpers.hpp"

// Create stream buffers and task stubs
UsbTest::UsbTest() {
  intf::usb::rx_stream_buffer.handle =
    xStreamBufferCreate(intf::usb::rx_stream_buffer.size, 1uz);
  intf::usb::tx_stream_buffer.handle =
    xStreamBufferCreate(intf::usb::tx_stream_buffer.size, 1uz);
  task_create_stub(mw::ulf::dcc_ein::task.handle);
  task_create_stub(mw::ulf::decup_ein::task.handle);
  task_create_stub(mw::ulf::susiv2::task.handle);
}

// Delete stream buffers and task stubs
UsbTest::~UsbTest() {
  stream_buffer_delete_clear_handle(intf::usb::rx_stream_buffer.handle);
  stream_buffer_delete_clear_handle(intf::usb::tx_stream_buffer.handle);
  task_delete_clear_handle(mw::ulf::dcc_ein::task.handle);
  task_delete_clear_handle(mw::ulf::decup_ein::task.handle);
  task_delete_clear_handle(mw::ulf::susiv2::task.handle);
}
