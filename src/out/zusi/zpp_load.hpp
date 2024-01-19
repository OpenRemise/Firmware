///
///
/// \file   out/zusi/zpp_load.hpp
/// \author Vincent Hamp
/// \date   31/03/2023

#pragma once

#include <zusi/zusi.hpp>

namespace out::zusi {

class ZppLoad final : public ::zusi::tx::Base {
  void transmitBytes(std::span<uint8_t const> chunk,
                     ::zusi::Mbps mbps) const final;
  void spiMaster() const final;
  void gpioInput() const final;
  void gpioOutput() const final;
  void writeClock(bool state) const final;
  void writeData(bool state) const final;
  bool readData() const final;
  void delayUs(uint32_t us) const final;
};

}  // namespace out::zusi