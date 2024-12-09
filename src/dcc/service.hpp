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

#pragma once

#include <mutex>
#include <optional>
#include <z21/z21.hpp>
#include "http/endpoints.hpp"
#include "locos.hpp"

namespace dcc {

class Service : public z21::server::intf::Dcc {
public:
  explicit Service(BaseType_t xCoreID);
  ~Service();

  //
  void z21(std::shared_ptr<z21::server::intf::System> z21_system_service,
           std::shared_ptr<z21::server::intf::Dcc> z21_dcc_service);

  //
  http::Response locosGetRequest(http::Request const& req);
  http::Response locosDeleteRequest(http::Request const& req);
  http::Response locosPutRequest(http::Request const& req);

private:
  // This gets called by FreeRTOS
  void taskFunction(void*);

  void operationsLoop();
  void operationsDcc();
  void operationsBiDi();

  void serviceLoop();
  std::optional<uint8_t> serviceRead(uint16_t cv_addr);
  std::optional<uint8_t> serviceWrite(uint16_t cv_addr, uint8_t byte);
  std::optional<bool> serviceReceiveBit();
  std::optional<uint8_t> serviceReceiveByte(bool bit_verify_to_1);

  void sendToFront(Packet const& packet, size_t n = 1uz);
  void sendToBack(Packet const& packet, size_t n = 1uz);

  // Driving interface
  z21::LocoInfo::Mode locoMode(uint16_t addr) final;
  void locoMode(uint16_t, z21::LocoInfo::Mode mode) final;
  void function(uint16_t addr, uint32_t mask, uint32_t state) final;
  void drive(uint16_t addr,
             z21::LocoInfo::SpeedSteps speed_steps,
             uint8_t rvvvvvvv) final;
  z21::LocoInfo locoInfo(uint16_t addr) final;
  void broadcastLocoInfo(uint16_t addr) final;

  // Programming interface
  [[nodiscard]] bool cvRead(uint16_t cv_addr) final;
  [[nodiscard]] bool cvWrite(uint16_t cv_addr, uint8_t byte) final;
  void cvPomRead(uint16_t addr, uint16_t cv_addr) final;
  void cvPomWrite(uint16_t addr, uint16_t cv_addr, uint8_t byte) final;
  void cvNackShortCircuit() final;
  void cvNack() final;
  void cvAck(uint16_t cv_addr, uint8_t byte) final;

  void resume();
  void suspend();

  uint8_t programPacketCount() const;

  Locos _locos;
  std::mutex _internal_mutex;
  std::shared_ptr<z21::server::intf::System> _z21_system_service;
  std::shared_ptr<z21::server::intf::Dcc> _z21_dcc_service;
  uint8_t _priority_count{Loco::min_priority};

  //
  struct CvRequest {
    TickType_t then{};             ///<
    uint16_t addr{};               ///<
    uint16_t cv_addr{};            ///<
    std::optional<uint8_t> byte{}; ///<
  };
  ztl::inplace_deque<CvRequest, Z21_SERVER_MAX_LOCO_ADDRESSES_PER_CLIENT>
    _cv_request_deque{};
  ztl::inplace_deque<CvRequest, Z21_SERVER_MAX_LOCO_ADDRESSES_PER_CLIENT>
    _cv_pom_request_deque{};
};

} // namespace dcc