#include "service.hpp"
#include <ArduinoJson.h>
#include <esp_task.h>
#include <dcc/dcc.hpp>
#include <ranges>
#include "log.h"
#include "mem/nvs/locos.hpp"
#include "utility.hpp"

namespace dcc {

using namespace std::literals;

/// TODO
Service::Service() {
  assert(xTaskCreatePinnedToCore(make_tramp(this, &Service::taskFunction),
                                 task.name,
                                 task.stack_depth,
                                 NULL,
                                 task.priority,
                                 &task.handle,
                                 1));
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
      case Mode::DCCOperations: OperationsService::loop(); break;
      case Mode::DCCService: ServiceService::loop(); break;
      default: return;
    }
    vTaskDelay(pdMS_TO_TICKS(100u));
  }
}

}  // namespace dcc