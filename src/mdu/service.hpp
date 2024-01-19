#pragma once

#include <esp_task.h>
#include <queue>
#include "firmware_service.hpp"
#include "zpp_service.hpp"

namespace mdu {

class Service : public FirmwareService, public ZppService {
public:
  explicit Service();
  ~Service();

private:
  // This gets called by FreeRTOS
  void taskFunction(void*);

  void loop();
};

}  // namespace mdu