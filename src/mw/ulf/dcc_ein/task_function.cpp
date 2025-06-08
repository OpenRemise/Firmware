// Copyright (C) 2025 Vincent Hamp
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

/// ULF_DCC_EIN task function
///
/// \file   ulf/dcc_ein/task_function.cpp
/// \author Vincent Hamp
/// \date   04/05/2025

#include "task_function.hpp"
#include <array>
#include <charconv>
#include <cstring>
#include "intf/usb/tx_task_function.hpp"
#include "log.h"
#include "utility.hpp"

namespace ulf::dcc_ein {

/// Receive DCC packet from senddcc string
///
/// \retval dcc::Packet created from senddcc string
/// \retval std::nullopt on timeout
std::optional<dcc::Packet> receive_dcc_packet() {
  std::array<char, usb::rx_stream_buffer.size> stack;
  size_t count{};

  for (;;) {
    // Receive single character
    auto const bytes_received{
      xStreamBufferReceive(usb::rx_stream_buffer.handle,
                           &stack[count],
                           1uz,
                           pdMS_TO_TICKS(task.timeout))};
    count += bytes_received;
    if (!bytes_received || count > size(stack)) return std::nullopt;

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

/// Acknowledge senddcc string
void ack_senddcc_str() {
  auto const space_used{
    drv::out::tx_message_buffer.size -
    xMessageBufferSpacesAvailable(drv::out::tx_message_buffer.front_handle)};
  auto const str{::ulf::dcc_ein::ack2senddcc_str('b', space_used)};
  xStreamBufferSend(usb::tx_stream_buffer.handle,
                    data(str),
                    size(str),
                    pdMS_TO_TICKS(usb::tx_task.timeout));
}

/// Send DCC packet to drv::out::tx_message_buffer front
///
/// \param  packet  DCC packet
void send_to_front(dcc::Packet const& packet) {
  xMessageBufferSend(drv::out::tx_message_buffer.front_handle,
                     data(packet),
                     size(packet),
                     portMAX_DELAY);
}

/// Send DCC packet to drv::out::tx_message_buffer back
///
/// \param  packet  DCC packet
void send_to_back(dcc::Packet const& packet) {
  xMessageBufferSend(drv::out::tx_message_buffer.back_handle,
                     data(packet),
                     size(packet),
                     portMAX_DELAY);
}

/// Send DCC idle packets to drv::out::tx_message_buffer back
void send_idle_packets_to_back() {
  static constexpr auto idle_packet{dcc::make_idle_packet()};
  while (
    xMessageBufferSpacesAvailable(drv::out::tx_message_buffer.back_handle) >
    drv::out::tx_message_buffer.size * 0.5)
    send_to_back(idle_packet);
}

/// Receive addressed datagram
///
/// \retval AddressedDatagram received from out::track::rx_queue
/// \retval std::nullopt on timeout
std::optional<ulf::dcc_ein::AddressedDatagram> receive_addressed_datagram() {
  drv::out::track::RxQueue::value_type item;
  if (xQueueReceive(
        drv::out::track::rx_queue.handle, &item, pdMS_TO_TICKS(task.timeout)))
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
  xStreamBufferSend(usb::tx_stream_buffer.handle,
                    data(str),
                    size(str),
                    pdMS_TO_TICKS(usb::tx_task.timeout));
}

/// Actual ulf::dcc_ein::task loop
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

/// ULF_DCC_EIN task function
///
/// This task is created by the \ref usb::rx_task_function "USB receive task"
/// when a `DCC_EIN\r` protocol string is received. Once running the task scans
/// the CDC character stream for strings with pattern
/// `senddcc([\d0-9a-fA-F]{2})+\r`, converts them to DCC packets and then
/// transmits them to drv::out::tx_message_buffer.
///
/// If no further senddcc string is received before the \ref
/// mem::nvs::Settings::getHttpReceiveTimeout "HTTP receive timeout", the \ref
/// usb::rx_task_function "USB receive task" is resumed and this task destroys
/// itself.
void task_function(void*) {
  // Switch to ULF_DCC_EIN mode, preload packets
  if (auto expected{State::Suspended};
      state.compare_exchange_strong(expected, State::ULF_DCC_EIN)) {
    usb::transmit_ok();
    send_idle_packets_to_back();
    LOGI_TASK_RESUME(drv::out::track::dcc::task);
    loop();
  }
  // ... or not
  else
    usb::transmit_not_ok();

  LOGI_TASK_RESUME(usb::rx_task);
  LOGI_TASK_DESTROY();
}

} // namespace ulf::dcc_ein
