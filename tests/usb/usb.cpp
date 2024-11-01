#include "usb/rx_task_function.hpp"
#include "usb_test.hpp"

// UsbTest ctor starts all protocol tasks, so any_protocol_task_active must
// return true
TEST_F(UsbTest, all_protocol_tasks_active) {
  EXPECT_LT(eTaskGetState(usb::dcc_ein::task.handle), eSuspended);
  EXPECT_LT(eTaskGetState(usb::decup_ein::task.handle), eSuspended);
  EXPECT_LT(eTaskGetState(usb::susiv2::task.handle), eSuspended);
  EXPECT_TRUE(usb::any_protocol_task_active());
}

// After suspending all protocol tasks any_protocol_task_active must return
// false
TEST_F(UsbTest, all_protocol_tasks_suspended) {
  vTaskSuspend(usb::dcc_ein::task.handle);
  vTaskSuspend(usb::decup_ein::task.handle);
  vTaskSuspend(usb::susiv2::task.handle);
  EXPECT_EQ(eTaskGetState(usb::dcc_ein::task.handle), eSuspended);
  EXPECT_EQ(eTaskGetState(usb::decup_ein::task.handle), eSuspended);
  EXPECT_EQ(eTaskGetState(usb::susiv2::task.handle), eSuspended);
  EXPECT_FALSE(usb::any_protocol_task_active());
}