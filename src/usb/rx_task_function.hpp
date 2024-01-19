/// USB receive task function
///
/// \file   usb/rx_task_function.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <string_view>

namespace usb {

bool any_protocol_task_active();
std::string_view receive_command();
void rx_task_function(void*);

}  // namespace usb