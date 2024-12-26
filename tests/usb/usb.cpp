#include "usb/rx_task_function.hpp"
#include "usb_test.hpp"

// UsbTest ctor starts all protocol tasks, so any_protocol_task_active must
// return true
TEST_F(UsbTest, all_protocol_tasks_active) {
  EXPECT_LT(eTaskGetState(usb::ulf_dcc_ein::task.handle), eSuspended);
  EXPECT_LT(eTaskGetState(usb::ulf_decup_ein::task.handle), eSuspended);
  EXPECT_LT(eTaskGetState(usb::ulf_susiv2::task.handle), eSuspended);
  EXPECT_TRUE(usb::any_protocol_task_active());
}

// After suspending all protocol tasks any_protocol_task_active must return
// false
TEST_F(UsbTest, all_protocol_tasks_suspended) {
  vTaskSuspend(usb::ulf_dcc_ein::task.handle);
  vTaskSuspend(usb::ulf_decup_ein::task.handle);
  vTaskSuspend(usb::ulf_susiv2::task.handle);
  EXPECT_EQ(eTaskGetState(usb::ulf_dcc_ein::task.handle), eSuspended);
  EXPECT_EQ(eTaskGetState(usb::ulf_decup_ein::task.handle), eSuspended);
  EXPECT_EQ(eTaskGetState(usb::ulf_susiv2::task.handle), eSuspended);
  EXPECT_FALSE(usb::any_protocol_task_active());
}