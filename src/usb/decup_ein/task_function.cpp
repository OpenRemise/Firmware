// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// DECUP_EIN protocol task function
///
/// \file   usb/decup_ein/task_function.cpp
/// \author Vincent Hamp
/// \date   10/08/2024

#include "task_function.hpp"
#include <decup/decup.hpp>
#include <expected>
#include <ulf/decup_ein.hpp>
#include "../tx_task_function.hpp"
#include "log.h"
#include "utility.hpp"

namespace usb::decup_ein {

using namespace std::literals;
using ::ulf::decup_ein::ack, ::ulf::decup_ein::nak;

namespace {

/// Exclusive disjunction (ex-or)
///
/// \param  bytes Bytes to calculate ex-or for
/// \return Ex-or
constexpr uint8_t exor(std::span<uint8_t const> bytes) {
  return std::accumulate(cbegin(bytes),
                         cend(bytes),
                         static_cast<uint8_t>(0u),
                         [](uint8_t a, uint8_t b) { return a ^ b; });
}

/// Exclusive disjunction (ex-or)
///
/// \param  packet  Packet
/// \return Ex-or
constexpr uint8_t exor(::decup::Packet const& packet) {
  return exor({cbegin(packet), size(packet)});
}

//
class Base {
public:
  std::optional<uint8_t> receive(uint8_t byte) {
    std::optional<uint8_t> retval{};

    switch (_state) {
      // -.-
      case Entry:
        if ("DECUP_EIN\r"sv.contains(byte)) break;
        _state = Preamble;
        [[fallthrough]];

      //
      case Preamble:
        //
        if (byte == 0xEFu || byte == 0xBFu) {
          transmit({&byte, sizeof(byte)});
          break;
        }
        //
        _state = Startbyte;
        [[fallthrough]];

      // Doppelpuls -> Blockcount
      case Startbyte:
        if (transmit({&byte, sizeof(byte)}) == 2u) {
          retval = ack;
          _state = Blockcount;
          _decoder_id = byte;
        }
        break;

      // Einfachpuls -> SecurityByte1
      case Blockcount:
        if (transmit({&byte, sizeof(byte)}) == 1u) {
          retval = ack;
          _state = SecurityByte1;
          assert(byte > 8u + 1u);
          _block_count =
            (byte - 8u + 1u) *  // bootloader size is 8 blocks
            (256u / (std::ranges::contains(_ids_which_use_32b, _decoder_id)
                       ? 32uz
                       : 64uz));
        }
        break;

      // Einfachpuls -> SecurityByte2
      case SecurityByte1:
        //
        if (byte != 0x55u) reset();
        //
        else if (transmit({&byte, sizeof(byte)}) == 1u) {
          retval = ack;
          _state = SecurityByte2;
        }
        break;

      // Einfachpuls -> Data32/64
      case SecurityByte2:
        if (byte != 0xAAu) reset();
        //
        else if (transmit({&byte, sizeof(byte)}) == 1u) {
          retval = ack;
          _state = Data;
        }
        break;

      // Doppelpuls -> nächstes Paket
      // Einfachpuls -> letztes Paket wiederholen
      case Data:
        _packet.push_back(byte);
        if (size(_packet) <
              (std::ranges::contains(_ids_which_use_32b, _decoder_id) ? 34uz
                                                                      : 66uz) ||
            exor(_packet))
          break;
        else if (transmit({cbegin(_packet), size(_packet)}) == 2u) retval = ack;
        _packet.clear();
        break;
    }

    return retval;
  }

private:
  virtual uint8_t transmit(std::span<uint8_t const> bytes) = 0;

  void reset() {
    _packet.clear();
    _state = Entry;
    _block_count = _decoder_id = 0u;
  }

  /// IDs 200-205 uses 32b packets, all other IDs use 64b
  static constexpr std::array<uint8_t, 5uz> _ids_which_use_32b{
    200u, 202u, 203u, 204u, 205u};

  ::decup::Packet _packet{};

  enum : uint8_t {
    Entry,
    Preamble,
    Startbyte,
    Blockcount,
    SecurityByte1,
    SecurityByte2,
    Data,
  } _state{};

  size_t _block_count{};
  uint8_t _decoder_id{};
};

class ZsuLoad : public Base {
  uint8_t transmit(std::span<uint8_t const> bytes) final {
    // out::track::decup
    xMessageBufferSend(out::tx_message_buffer.front_handle,
                       data(bytes),
                       size(bytes),
                       pdMS_TO_TICKS(task.timeout));

    uint8_t acks;
    auto const bytes_received{
      xMessageBufferReceive(out::rx_message_buffer.handle,
                            &acks,
                            sizeof(acks),
                            pdMS_TO_TICKS(task.timeout))};
    assert(bytes_received);

    // for (auto c : bytes) printf("%X ", c);
    // printf(" -> %d\n", acks);

    return acks;
  }
};

/// \todo document
void transmit_response(uint8_t byte) {
  xStreamBufferSend(
    tx_stream_buffer.handle, &byte, sizeof(byte), pdMS_TO_TICKS(task.timeout));
}

/// Actual usb::decup_ein::task loop
void loop() {
  // auto const timeout{usb_receive_timeout2ms()};
  auto const timeout{task.timeout};
  TickType_t then{xTaskGetTickCount() + pdMS_TO_TICKS(timeout)};

  ZsuLoad zsu_load{};

  for (;;) {
    // Return on timeout
    if (auto const now{xTaskGetTickCount()}; now >= then) return;
    // In case we got a byte, reset timeout
    else if (uint8_t byte; xStreamBufferReceive(rx_stream_buffer.handle,
                                                &byte,
                                                sizeof(byte),
                                                pdMS_TO_TICKS(task.timeout))) {
      then = now + pdMS_TO_TICKS(timeout);
      if (auto const resp{zsu_load.receive(byte)}) transmit_response(*resp);
    }
  }
}

}  // namespace

/// \todo document
void task_function(void*) {
  for (;;) {
    LOGI_TASK_SUSPEND(task.handle);

    //
    if (auto expected{State::Suspended};
        state.compare_exchange_strong(expected, State::DECUP_EIN)) {
      transmit_ok();
      LOGI_TASK_RESUME(out::track::decup::task.handle);
      loop();
    }
    //
    else
      transmit_not_ok();

    LOGI_TASK_RESUME(usb::rx_task.handle);
  }
}

}  // namespace usb::decup_ein