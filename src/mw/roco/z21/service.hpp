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

#pragma once

#include <mutex>
#include <set>
#include <z21/z21.hpp>
#include "intf/http/message.hpp"

namespace mw::roco::z21 {

using namespace ::z21;

using ServerBase = z21::server::Base<z21::server::intf::System,
                                     z21::server::intf::Dcc,
                                     z21::server::intf::Settings>;

class Service : public ServerBase {
public:
  Service();

  void dcc(std::shared_ptr<z21::server::intf::Dcc> dcc_service);

  esp_err_t socket(intf::http::Message& msg);

private:
  // This gets called by FreeRTOS
  [[noreturn]] void taskFunction(void*);

  //
  void transmit(z21::Socket const& sock,
                std::span<uint8_t const> datasets) final;

  //
  [[nodiscard]] bool trackPower(bool on) final;
  [[nodiscard]] bool stop() final;
  [[nodiscard]] int32_t serialNumber() const final;
  void logoff(z21::Socket const& sock) final;

  // Driving interface
  z21::LocoInfo locoInfo(uint16_t loco_addr) final;
  void locoFunction(uint16_t loco_addr, uint32_t mask, uint32_t state) final;
  void locoDrive(uint16_t loco_addr,
                 z21::LocoInfo::SpeedSteps speed_steps,
                 uint8_t rvvvvvvv) final;
  z21::LocoInfo::Mode locoMode(uint16_t loco_addr) final;
  void locoMode(uint16_t loco_addr, z21::LocoInfo::Mode mode) final;

  // Switching interface
  [[nodiscard]] z21::TurnoutInfo turnoutInfo(uint16_t accy_addr) final;
  [[nodiscard]] z21::AccessoryInfo accessoryInfo(uint16_t accy_addr) final;
  void turnout(uint16_t accy_addr, bool p, bool a, bool q) final;
  void accessory(uint16_t accy_addr, uint8_t dddddddd) final;
  [[nodiscard]] z21::TurnoutInfo::Mode turnoutMode(uint16_t accy_addr) final;
  void turnoutMode(uint16_t accy_addr, z21::TurnoutInfo::Mode mode) final;

  // Programming interface
  [[nodiscard]] bool cvRead(uint16_t cv_addr) final;
  [[nodiscard]] bool cvWrite(uint16_t cv_addr, uint8_t byte) final;
  void cvPomRead(uint16_t loco_addr, uint16_t cv_addr) final;
  void cvPomWrite(uint16_t loco_addr, uint16_t cv_addr, uint8_t byte) final;
  void cvPomAccessoryRead(uint16_t accy_addr, uint16_t cv_addr) final;
  void
  cvPomAccessoryWrite(uint16_t accy_addr, uint16_t cv_addr, uint8_t byte) final;

  // RailCom interface
  [[nodiscard]] z21::RailComData railComData(uint16_t loco_addr) final;

  // Settings interface
  [[nodiscard]] z21::CommonSettings commonSettings() final;
  void commonSettings(z21::CommonSettings const& common_settings) final;
  [[nodiscard]] z21::MmDccSettings mmDccSettings() final;
  void mmDccSettings(z21::MmDccSettings const& mm_dcc_settings) final;

  // Logging interface
  // void log(char const* str) final;

  //
  bool trackPower(bool on, State dcc_state);

  ///
  std::shared_ptr<z21::server::intf::Dcc> _dcc_service;

  ///
  std::set<int> _ws_sock_fds;

  std::mutex _internal_mutex;
};

} // namespace mw::roco::z21
