// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "service.hpp"
#include <zusi/zusi.hpp>
#include "log.h"
#include "utility.hpp"

namespace mdu {

/// \todo document
Service::Service(BaseType_t xCoreID) {
  if (!xTaskCreatePinnedToCore(make_tramp(this, &Service::taskFunction),
                               task.name,
                               task.stack_size,
                               NULL,
                               task.priority,
                               &task.handle,
                               xCoreID))
    assert(false);
}

/// \todo document
Service::~Service() {
  if (task.handle) vTaskDelete(task.handle);
}

/// \todo document
void Service::taskFunction(void*) {
  for (;;) switch (state.load()) {
      case State::MDUZpp:
        ZppService::loop();
        vTaskDelay(pdMS_TO_TICKS(100u));
        break;
      case State::MDUZsu:
        ZsuService::loop();
        vTaskDelay(pdMS_TO_TICKS(100u));
        break;
      default: LOGI_TASK_SUSPEND(task.handle); break;
    }
}

}  // namespace mdu