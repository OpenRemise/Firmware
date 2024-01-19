#include "task_function.hpp"
#include "log.h"
#include "resume.hpp"
#include "suspend.hpp"

namespace out::track::mdu {

/// TODO
void task_function(void*) {
  for (;;) { LOGI_TASK_SUSPEND(task.handle); }
}

}  // namespace out::track::mdu