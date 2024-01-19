#pragma once

#include <mutex>
#include <optional>
#include "operations_service.hpp"
#include "service_service.hpp"

namespace dcc {

class Service : public OperationsService, public ServiceService {
public:
  explicit Service();
  ~Service();

private:
  // This gets called by FreeRTOS
  void taskFunction(void*);

  void loop();
};

}  // namespace dcc