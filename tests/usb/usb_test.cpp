#include "usb_test.hpp"
#include "freertos_helpers.hpp"

// Create stream buffers and task stubs
UsbTest::UsbTest() {
  usb::rx_stream_buffer.handle =
    xStreamBufferCreate(usb::rx_stream_buffer.size, 1uz);
  usb::tx_stream_buffer.handle =
    xStreamBufferCreate(usb::tx_stream_buffer.size, 1uz);
  task_create_stub(usb::dcc_ein::rx_task.handle);
  task_create_stub(usb::dcc_ein::tx_task.handle);
  task_create_stub(usb::susiv2::rx_task.handle);
  task_create_stub(usb::susiv2::tx_task.handle);
}

// Delete stream buffers and task stubs
UsbTest::~UsbTest() {
  stream_buffer_delete_clear_handle(usb::rx_stream_buffer.handle);
  stream_buffer_delete_clear_handle(usb::tx_stream_buffer.handle);
  task_delete_clear_handle(usb::dcc_ein::rx_task.handle);
  task_delete_clear_handle(usb::dcc_ein::tx_task.handle);
  task_delete_clear_handle(usb::susiv2::rx_task.handle);
  task_delete_clear_handle(usb::susiv2::tx_task.handle);
}