// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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