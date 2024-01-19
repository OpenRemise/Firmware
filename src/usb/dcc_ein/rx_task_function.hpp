/// DCC_EIN protocol receive task function
///
/// \file   usb/dcc_ein/rx_task_function.hpp
/// \author Vincent Hamp
/// \date   10/02/2023

#pragma once

#include <optional>
#include <span>

namespace usb::dcc_ein {

std::optional<std::span<char>> receive_senddcc_str(std::span<char> str);
void rx_task_function(void*);

}  // namespace usb::dcc_ein