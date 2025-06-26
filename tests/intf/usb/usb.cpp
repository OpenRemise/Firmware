#include "intf/usb/rx_task_function.hpp"
#include "usb_test.hpp"

TEST_F(UsbTest, no_service_tasks_active) {
  EXPECT_FALSE(intf::usb::any_service_task_active());
}

TEST_F(UsbTest, all_service_tasks_active) {
  mw::zimo::ulf::dcc_ein::task.create([](void*) {
    for (;;) vTaskDelay(pdMS_TO_TICKS(1000u));
  });
  mw::zimo::ulf::decup_ein::task.create([](void*) {
    for (;;) vTaskDelay(pdMS_TO_TICKS(1000u));
  });
  mw::zimo::ulf::susiv2::task.create([](void*) {
    for (;;) vTaskDelay(pdMS_TO_TICKS(1000u));
  });
  EXPECT_TRUE(intf::usb::any_service_task_active());
}
