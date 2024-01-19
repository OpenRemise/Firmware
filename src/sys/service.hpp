/// Cover /sys/ endpoint
///
/// \file   sys/service.hpp
/// \author Vincent Hamp
/// \date   13/08/2023

#pragma once

#include <esp_err.h>
#include "http/request.hpp"
#include "http/response.hpp"

namespace sys {

class Service {
public:
  http::Response getRequest(http::Request const& req);
  http::Response postRequest(http::Request const& req);
};

}  // namespace sys