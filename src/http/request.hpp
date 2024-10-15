// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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