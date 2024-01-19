/// USB transmit task function
///
/// \file   usb/tx_task_function.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

namespace usb {

void transmit_ok();
void transmit_not_ok();
void tx_task_function(void*);

}  // namespace usb