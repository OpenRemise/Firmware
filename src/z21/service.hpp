// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <set>
#include <z21/z21.hpp>
#include "http/message.hpp"

namespace z21 {

using ServerBase =
  z21::server::Base<z21::server::intf::System, z21::server::intf::Dcc>;

class Service : public ServerBase {
public:
  explicit Service(BaseType_t xCoreID);
  ~Service();

  void dcc(std::shared_ptr<z21::server::intf::Dcc> dcc_service);

  esp_err_t socket(http::Message& msg);

private:
  // This gets called by FreeRTOS
  void taskFunction(void*);

  //
  void transmit(z21::Socket const& sock,
                std::span<uint8_t const> datasets) final;

  //
  [[nodiscard]] bool trackPower(bool on) final;
  [[nodiscard]] bool stop() final;
  void logoff(z21::Socket const& sock) final;

  // Driving interface
  z21::LocoInfo::Mode locoMode(uint16_t addr) final;
  void locoMode(uint16_t addr, z21::LocoInfo::Mode mode) final;
  void function(uint16_t addr, uint32_t mask, uint32_t state) final;
  void drive(uint16_t addr,
             z21::LocoInfo::SpeedSteps speed_steps,
             uint8_t rvvvvvvv) final;
  z21::LocoInfo locoInfo(uint16_t addr) final;

  // Programming interface
  [[nodiscard]] bool cvRead(uint16_t cv_addr) final;
  [[nodiscard]] bool cvWrite(uint16_t cv_addr, uint8_t byte) final;
  void cvPomRead(uint16_t addr, uint16_t cv_addr) final;
  void cvPomWrite(uint16_t addr, uint16_t cv_addr, uint8_t byte) final;

  //
  bool trackPower(bool on, State dcc_state);

  ///
  std::shared_ptr<z21::server::intf::Dcc> _dcc_service;

  ///
  std::set<int> _ws_sock_fds;

  std::mutex _internal_mutex;
};

}  // namespace z21