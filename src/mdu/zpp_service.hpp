#pragma once

#include <esp_task.h>
#include "http/message.hpp"

namespace mdu {

class ZppService {
public:
  esp_err_t zppSocket(http::Message& msg);

protected:
  void loop();
};

}  // namespace mdu