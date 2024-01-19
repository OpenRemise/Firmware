#pragma once

#include <esp_task.h>
#include <queue>
#include <ztl/inplace_vector.hpp>
#include <zusi/zusi.hpp>
#include "http/message.hpp"

namespace zusi {

class Service {
public:
  explicit Service();
  ~Service();

  esp_err_t socket(http::Message& msg);

private:
  // This gets called by FreeRTOS
  void taskFunction(void*);

  void loop();
  ztl::inplace_vector<uint8_t, 8uz - 1uz>
  transmit(std::vector<uint8_t> const& payload) const;
  void close();

  ztl::inplace_vector<uint8_t, 8uz - 1uz> _data{};
  std::queue<http::Message> _queue{};
};

}  // namespace zusi