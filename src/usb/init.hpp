/// Initialize USB
///
/// \file   usb/init.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_err.h>

namespace usb {

esp_err_t init();

}  // namespace usb