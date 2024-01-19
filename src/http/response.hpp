/// HTTP response
///
/// \file   http/response.hpp
/// \author Vincent Hamp
/// \date   01/03/2023

#pragma once

#include <expected>
#include <string>

namespace http {

using Response = std::expected<std::string, std::string>;

}  // namespace http