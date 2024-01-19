/// Cover /settings/ endpoint
///
/// \file   settings/service.hpp
/// \author Vincent Hamp
/// \date   15/05/2023

#pragma once

#include <esp_err.h>
#include "http/request.hpp"
#include "http/response.hpp"

namespace settings {

class Service {
public:
  http::Response getRequest(http::Request const& req);
  http::Response postRequest(http::Request const& req);
};

}  // namespace settings