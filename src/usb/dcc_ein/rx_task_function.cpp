/// DCC_EIN protocol receive task function
///
/// \file   usb/dcc_ein/rx_task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "rx_task_function.hpp"
#include <array>
#include <charconv>
#include <cstring>
#include <dcc_ein/dcc_ein.hpp>
#include "../tx_task_function.hpp"
#include "log.h"

namespace usb::dcc_ein {

/// Receive senddcc string
///
/// String should have the pattern 'senddcc( [0-9a-fA-F]{2}){3,}\r'. This isn't
/// enforced though and the function simply reads until receiving a '\r'
/// character.
///
/// \param  str senddcc string
/// \return Received senddcc string
/// \return std::nullopt on timeout
std::optional<std::span<char>> receive_senddcc_str(std::span<char> str) {
  size_t count{};
  do {
    auto const bytes_received{
      xStreamBufferReceive(rx_stream_buffer.handle,
                           &str[count],
                           1uz,
                           pdMS_TO_TICKS(rx_task.timeout))};
    count += bytes_received;
    if (!bytes_received || count >= size(str)) return std::nullopt;
  } while (str[count - 1uz] != '\r');
  return str.subspan(0uz, count);
}

namespace {

/// Transmit raw bytes of DCC packet to out::tx_message_buffer
///
/// \param  packet  DCC packet
void transmit(dcc::Packet const& packet) {
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(packet),
                     size(packet),
                     portMAX_DELAY);
}

/// TODO
void ack_senddcc_str() {
  auto const space_used{
    out::tx_message_buffer.size -
    xMessageBufferSpacesAvailable(out::tx_message_buffer.front_handle)};
  auto const str{::dcc_ein::rx::ack2senddcc_str('b', space_used)};
  xStreamBufferSend(tx_stream_buffer.handle,
                    data(str),
                    size(str),
                    pdMS_TO_TICKS(tx_task.timeout));
}

/// Actual usb::dcc_ein::rx_task loop
void loop() {
  std::array<char, rx_stream_buffer.size> stack;
  while (auto const str{receive_senddcc_str(stack)}) {
    LOGD("%.*s", size(*str), data(*str));
    if (auto const packet{::dcc_ein::rx::senddcc_str2packet(
          std::string_view{cbegin(*str), cend(*str)})}) {
      transmit(*packet);
      ack_senddcc_str();
    }
  }
}

}  // namespace

/// DCC_EIN receive task function
///
/// Immediatly suspends itself after creation. It's only resumed after
/// usb::rx_task_function receives a "DCC_EIN\r" protocol entry string. Once
/// running scan the CDC character stream for strings with pattern `senddcc(
/// [\d0-9a-fA-F]{2})+\r` and transmit the data to out::tx_message_buffer.
void rx_task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(rx_task.handle);
    if (auto expected{Mode::Suspended};
        mode.compare_exchange_strong(expected, Mode::DCC_EIN)) {
      transmit_ok();
      LOGI_TASK_RESUME(out::track::dcc::task.handle);
      loop();
    } else transmit_not_ok();
    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

}  // namespace usb::dcc_ein