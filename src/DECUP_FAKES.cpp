// absichtlich ka lizenz...
#include <cstdio>
#include <gsl/util>
#include <ulf/decup_ein.hpp>

// Fake MX645 sound update (with CRC instead of XOR)
uint8_t DECUP_TRANSMIT_ZPP_FAKE(std::span<uint8_t const> bytes) {
  static decup::Packet _retvals{};
  uint8_t acks{};
  gsl::final_action print_outgoing{[&] { printf(" -> %d\n", acks); }};

  // Print incoming
  for (auto c : bytes) printf("%02X ", c);

  if (size(bytes) == 1uz) {
    // Preamble
    if (bytes[0uz] == 0xEFu || bytes[0uz] == 0xBFu) return 0uz;
  }

  // ID
  if (size(bytes) == 1uz && bytes[0uz] == 4u) {
    _retvals.clear();
    _retvals.push_back(221u & 0x80u ? 2u : 1u);
    _retvals.push_back(221u & 0x40u ? 2u : 1u);
    _retvals.push_back(221u & 0x20u ? 2u : 1u);
    _retvals.push_back(221u & 0x10u ? 2u : 1u);
    _retvals.push_back(221u & 0x08u ? 2u : 1u);
    _retvals.push_back(221u & 0x04u ? 2u : 1u);
    _retvals.push_back(221u & 0x02u ? 2u : 1u);
    _retvals.push_back(221u & 0x01u ? 2u : 1u);
  }
  // CRC or XOR
  else if (size(bytes) == 1uz && bytes[0uz] == 7u) {
    _retvals.clear();
    uint8_t crc{1u};
    _retvals.push_back(crc & 0x80u ? 2u : 1u);
    _retvals.push_back(crc & 0x40u ? 2u : 1u);
    _retvals.push_back(crc & 0x20u ? 2u : 1u);
    _retvals.push_back(crc & 0x10u ? 2u : 1u);
    _retvals.push_back(crc & 0x08u ? 2u : 1u);
    _retvals.push_back(crc & 0x04u ? 2u : 1u);
    _retvals.push_back(crc & 0x02u ? 2u : 1u);
    _retvals.push_back(crc & 0x01u ? 2u : 1u);
  }
  // CV lesen
  else if (size(bytes) == 3uz && bytes[0uz] == 1u) {
    _retvals.clear();
    // CV7
    if (bytes[2uz] == 6u) {
      uint8_t version{40u};
      _retvals.push_back(version & 0x80u ? 2u : 1u);
      _retvals.push_back(version & 0x40u ? 2u : 1u);
      _retvals.push_back(version & 0x20u ? 2u : 1u);
      _retvals.push_back(version & 0x10u ? 2u : 1u);
      _retvals.push_back(version & 0x08u ? 2u : 1u);
      _retvals.push_back(version & 0x04u ? 2u : 1u);
      _retvals.push_back(version & 0x02u ? 2u : 1u);
      _retvals.push_back(version & 0x01u ? 2u : 1u);
    }
    // CV8
    else if (bytes[2uz] == 7u) {
      _retvals.push_back(145u & 0x80u ? 2u : 1u);
      _retvals.push_back(145u & 0x40u ? 2u : 1u);
      _retvals.push_back(145u & 0x20u ? 2u : 1u);
      _retvals.push_back(145u & 0x10u ? 2u : 1u);
      _retvals.push_back(145u & 0x08u ? 2u : 1u);
      _retvals.push_back(145u & 0x04u ? 2u : 1u);
      _retvals.push_back(145u & 0x02u ? 2u : 1u);
      _retvals.push_back(145u & 0x01u ? 2u : 1u);
    }
  }
  // Flash erase
  else if (size(bytes) == 4uz && bytes[0uz] == 3u)
    acks = 1u;
  // CV write
  else if ((size(bytes) == 5uz && bytes[0uz] == 0x02u) ||
           (size(bytes) == 6uz && bytes[0uz] == 0x06u))
    acks = 1u;
  // Flash write
  else if (size(bytes) == DECUP_MAX_PACKET_SIZE) acks = 2u;

  if (size(_retvals)) {
    acks = _retvals.back();
    _retvals.pop_back();
  }

  return acks;
}

// Fake MX645 decoder update
uint8_t DECUP_TRANSMIT_ZSU_FAKE(std::span<uint8_t const> bytes) {
  uint8_t acks{};
  gsl::final_action print_outgoing{[&] { printf(" -> %d\n", acks); }};

  // Print incoming
  for (auto c : bytes) printf("%02X ", c);

  if (size(bytes) == 1uz) {
    // Preamble
    if (bytes[0uz] == 0xEFu || bytes[0uz] == 0xBFu) {
      acks = 0uz;
      return acks;
    }
  }

  if (size(bytes) == 1uz) {
    // MX645 startbyte
    if (bytes[0uz] == 221u) acks = 2uz;
    else return acks = 1uz;
  }
  // all other packets
  else
    return acks = 2uz;

  return acks;
}