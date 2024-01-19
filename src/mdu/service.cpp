#include "service.hpp"
#include <zusi/zusi.hpp>
#include "log.h"
#include "utility.hpp"

namespace mdu {

/// TODO
Service::Service() {
  xTaskCreatePinnedToCore(make_tramp(this, &Service::taskFunction),
                          task.name,
                          task.stack_depth,
                          NULL,
                          task.priority,
                          &task.handle,
                          1);
}

/// TODO
Service::~Service() {
  if (task.handle) vTaskDelete(task.handle);
}

/// TODO
void Service::taskFunction(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);
    loop();
  }
}

/// TODO
void Service::loop() {
  for (;;) {
    switch (mode.load()) {
      case Mode::MDUFirmware: FirmwareService::loop(); break;
      case Mode::MDUZpp: ZppService::loop(); break;
      default: return;
    }
    vTaskDelay(pdMS_TO_TICKS(100u));
  }
}

}  // namespace mdu