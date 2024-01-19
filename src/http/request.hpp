/// HTTP request
///
/// \file   http/request.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <string>

namespace http {

struct Request {
  std::string uri;  // endpoint + query
  std::string body;
};

}  // namespace http