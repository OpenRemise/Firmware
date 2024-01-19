#include "usb/rx_task_function.hpp"
#include "usb_test.hpp"

TEST_F(UsbTest, receive_dcc_ein_protocol_entry) {
  std::string_view dcc_ein{"DCC_EIN\r"};
  xStreamBufferSend(
    usb::rx_stream_buffer.handle, data(dcc_ein), size(dcc_ein), 0u);
  EXPECT_EQ(usb::receive_command(), dcc_ein);
}

TEST_F(UsbTest, receive_decup_ein_protocol_entry) {
  std::string_view decup_ein{"DECUP_EIN\r"};
  xStreamBufferSend(
    usb::rx_stream_buffer.handle, data(decup_ein), size(decup_ein), 0u);
  EXPECT_EQ(usb::receive_command(), decup_ein);
}

TEST_F(UsbTest, receive_mdusndprep_protocol_entry) {
  std::string_view mdusndprep{"MDUSNDPREP\r"};
  xStreamBufferSend(
    usb::rx_stream_buffer.handle, data(mdusndprep), size(mdusndprep), 0u);
  EXPECT_EQ(usb::receive_command(), mdusndprep);
}

TEST_F(UsbTest, receive_susiv2_protocol_entry) {
  std::string_view susiv2{"SUSIV2\r"};
  xStreamBufferSend(
    usb::rx_stream_buffer.handle, data(susiv2), size(susiv2), 0u);
  EXPECT_EQ(usb::receive_command(), susiv2);
}

// UsbTest ctor starts all protocol tasks, so any_protocol_task_active must
// return true
TEST_F(UsbTest, all_protocol_tasks_active) {
  EXPECT_LT(eTaskGetState(usb::dcc_ein::rx_task.handle), eSuspended);
  EXPECT_LT(eTaskGetState(usb::dcc_ein::tx_task.handle), eSuspended);
  EXPECT_LT(eTaskGetState(usb::susiv2::rx_task.handle), eSuspended);
  EXPECT_LT(eTaskGetState(usb::susiv2::tx_task.handle), eSuspended);
  EXPECT_TRUE(usb::any_protocol_task_active());
}

// After suspending all protocol tasks any_protocol_task_active must return
// false
TEST_F(UsbTest, all_protocol_tasks_suspended) {
  vTaskSuspend(usb::dcc_ein::rx_task.handle);
  vTaskSuspend(usb::dcc_ein::tx_task.handle);
  vTaskSuspend(usb::susiv2::rx_task.handle);
  vTaskSuspend(usb::susiv2::tx_task.handle);
  EXPECT_EQ(eTaskGetState(usb::dcc_ein::rx_task.handle), eSuspended);
  EXPECT_EQ(eTaskGetState(usb::dcc_ein::tx_task.handle), eSuspended);
  EXPECT_EQ(eTaskGetState(usb::susiv2::rx_task.handle), eSuspended);
  EXPECT_EQ(eTaskGetState(usb::susiv2::tx_task.handle), eSuspended);
  EXPECT_FALSE(usb::any_protocol_task_active());
}