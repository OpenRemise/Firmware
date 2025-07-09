#include "dcc_ein_test.hpp"
#include "mw/zimo/ulf/dcc_ein/task_function.hpp"

TEST_F(UsbUlfDccEinTest, receive_timeout) {
  EXPECT_FALSE(mw::zimo::ulf::dcc_ein::receive_dcc_packet());
}

TEST_F(UsbUlfDccEinTest, receive_carriage_return_terminated_string) {
  std::string_view str{"senddcc 0A 0B 0C 0D 0E\r"};
  xStreamBufferSend(
    intf::usb::rx_stream_buffer.handle, data(str), size(str), 0u);
  EXPECT_TRUE(mw::zimo::ulf::dcc_ein::receive_dcc_packet());
}
