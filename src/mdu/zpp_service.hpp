// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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