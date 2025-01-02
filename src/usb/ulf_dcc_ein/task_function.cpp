// Copyright (C) 2024 Vincent Hamp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/// ULF_DCC_EIN protocol task function
///
/// \file   usb/ulf_dcc_ein/task_function.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "task_function.hpp"
#include <array>
#include <charconv>
#include <cstring>
#include <ulf/dcc_ein.hpp>
#include "../tx_task_function.hpp"
#include "log.h"
#include "utility.hpp"

namespace usb::ulf_dcc_ein {

/// Receive DCC packet from senddcc string
///
/// \retval dcc::Packet created from senddcc string
/// \retval std::nullopt on timeout
std::optional<dcc::Packet> receive_dcc_packet() {
  std::array<char, rx_stream_buffer.size> stack;
  size_t count{};

  for (;;) {
    // Receive single character
    auto const bytes_received{
      xStreamBufferReceive(rx_stream_buffer.handle,
                           &stack[count],
                           1uz,
                           pdMS_TO_TICKS(task.timeout))};
    count += bytes_received;
    if (!bytes_received || count >= size(stack)) return std::nullopt;

    // Convert senddcc string to DCC packet
    auto const packet{
      ulf::dcc_ein::senddcc_str2packet(std::string_view{cbegin(stack), count})};
    // Not enough characters
    if (!packet) count = 0uz;
    // Complete packet
    else if (*packet) return *packet;
  }
}

namespace {

/// Send DCC packet to out::tx_message_buffer front
///
/// \param  packet  DCC packet
void send_to_front(dcc::Packet const& packet) {
  xMessageBufferSend(out::tx_message_buffer.front_handle,
                     data(packet),
                     size(packet),
                     portMAX_DELAY);
}

/// Send DCC packet to out::tx_message_buffer back
///
/// \param  packet  DCC packet
void send_to_back(dcc::Packet const& packet) {
  xMessageBufferSend(out::tx_message_buffer.back_handle,
                     data(packet),
                     size(packet),
                     portMAX_DELAY);
}

/// Send DCC idle packets to out::tx_message_buffer back
void send_idle_packets_to_back() {
  static constexpr auto idle_packet{dcc::make_idle_packet()};
  while (xMessageBufferSpacesAvailable(out::tx_message_buffer.back_handle) >
         out::tx_message_buffer.size * 0.5)
    send_to_back(idle_packet);
}

/// \todo document
void ack_senddcc_str() {
  auto const space_used{
    out::tx_message_buffer.size -
    xMessageBufferSpacesAvailable(out::tx_message_buffer.front_handle)};
  auto const str{::ulf::dcc_ein::ack2senddcc_str('b', space_used)};
  xStreamBufferSend(tx_stream_buffer.handle,
                    data(str),
                    size(str),
                    pdMS_TO_TICKS(tx_task.timeout));
}

/// Receive addressed datagram
///
/// \retval AddressedDatagram received from out::track::rx_queue
/// \retval std::nullopt on timeout
std::optional<ulf::dcc_ein::AddressedDatagram> receive_addressed_datagram() {
  out::track::RxQueue::value_type item;
  if (xQueueReceive(
        out::track::rx_queue.handle, &item, pdMS_TO_TICKS(task.timeout)))
    return ulf::dcc_ein::AddressedDatagram{
      .addr = dcc::decode_address(item.packet), .datagram = item.datagram};
  else return std::nullopt;
}

/// Transmit addressed datagram
///
/// The addressed datagram is converted to a sendbidi string with pattern
/// `sendbidi [ubsalrtei][0-9a-fA-F]{4}( [0-9a-fA-F]{2}){8}\r` prior to
/// transmission. The string is then send to usb::tx_stream_buffer.
///
/// \param  addr_datagram AddressedDatagram received from out::track::rx_queue
void transmit_addressed_datagram(
  ulf::dcc_ein::AddressedDatagram const& addr_datagram) {
  auto const str{ulf::dcc_ein::addressed_datagram2sendbidi_str(addr_datagram)};
  xStreamBufferSend(
    tx_stream_buffer.handle, data(str), size(str), pdMS_TO_TICKS(task.timeout));
}

/// Actual usb::dcc_ein::rx_task loop
void loop() {
  auto const timeout{http_receive_timeout2ms()};
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(timeout)};

  for (;;) {
    send_idle_packets_to_back();

    // Return on timeout
    if (auto const now{xTaskGetTickCount()}; now >= then) return;
    // In case we got a packet, reset timeout
    else if (auto const packet{receive_dcc_packet()}) {
      then = now + pdMS_TO_TICKS(timeout);
      send_to_front(*packet);
      ack_senddcc_str();
    }

    // Transmit datagrams
    if (auto const addr_datagram{receive_addressed_datagram()};
        addr_datagram && std::ranges::any_of(addr_datagram->datagram,
                                             [](uint8_t b) { return b; }))
      transmit_addressed_datagram(*addr_datagram);
  }
}

} // namespace

/// ULF_DCC_EIN receive task function
///
/// Immediately suspends itself after creation. It's only resumed after
/// usb::rx_task_function receives a "DCC_EIN\r" protocol entry string. Once
/// running scan the CDC character stream for strings with pattern `senddcc(
/// [\d0-9a-fA-F]{2})+\r` and transmit the data to out::tx_message_buffer.
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    //
    if (auto expected{State::Suspended};
        state.compare_exchange_strong(expected, State::ULF_DCC_EIN)) {
      transmit_ok();
      send_idle_packets_to_back();
      LOGI_TASK_RESUME(out::track::dcc::task.handle);
      loop();
    }
    //
    else
      transmit_not_ok();

    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

} // namespace usb::ulf_dcc_ein