// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <esp_task.h>
#include <queue>
#include "zpp_service.hpp"
#include "zsu_service.hpp"

namespace mdu {

class Service : public ZppService, public ZsuService {
public:
  explicit Service(BaseType_t xCoreID);
  ~Service();

private:
  // This gets called by FreeRTOS
  void taskFunction(void*);
};

}  // namespace mdu