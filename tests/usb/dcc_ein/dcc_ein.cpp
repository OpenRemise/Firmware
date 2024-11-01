#include "dcc_ein_test.hpp"
#include "usb/dcc_ein/task_function.hpp"

TEST_F(UsbDccEinTest, receive_timeout) {
  EXPECT_FALSE(usb::dcc_ein::receive_dcc_packet());
}

TEST_F(UsbDccEinTest, receive_carriage_return_terminated_string) {
  std::string_view str{"senddcc 0A 0B 0C 0D 0E\r"};
  xStreamBufferSend(usb::rx_stream_buffer.handle, data(str), size(str), 0u);
  EXPECT_TRUE(usb::dcc_ein::receive_dcc_packet());
}
