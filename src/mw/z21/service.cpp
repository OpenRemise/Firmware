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

#include "service.hpp"
#include <lwip/sockets.h>
#include <ztl/string.hpp>
#include "drv/led/bug.hpp"
#include "log.h"
#include "utility.hpp"

namespace z21 {

using namespace std::literals;

/// \todo document
Service::Service() {
  task.create(ztl::make_trampoline(this, &Service::taskFunction));
}

/// \todo document
Service::~Service() { task.destroy(); }

/// \todo document
void Service::dcc(std::shared_ptr<z21::server::intf::Dcc> dcc_service) {
  _dcc_service = dcc_service;
}

/// \todo document
/// \bug if the socket closes for any reason we're fucked, there must be some
/// way to detect such cases and restart the socket in the Frontend?
esp_err_t Service::socket(http::Message& msg) {
  switch (msg.type) {
    case HTTPD_WS_TYPE_BINARY: {
      _ws_sock_fds.insert(msg.sock_fd);

      //
      sockaddr_in dest_addr_ip4;
      socklen_t socklen{sizeof(dest_addr_ip4)};
      if (getpeername(msg.sock_fd,
                      std::bit_cast<sockaddr*>(&dest_addr_ip4),
                      &socklen) < 0) {
        LOGD("getpeername failed %s", strerror(errno));
        return ESP_FAIL;
      }

      //
      std::lock_guard lock{_internal_mutex};
      receive({msg.sock_fd, std::bit_cast<sockaddr*>(&dest_addr_ip4), socklen},
              msg.payload);
      execute();
      break;
    }
    case HTTPD_WS_TYPE_CLOSE:
      /// \todo implicit power off here if there are no more clients registered?
      _ws_sock_fds.erase(msg.sock_fd);
      break;
    default: break;
  }

  return ESP_OK;
}

/// \todo document
void Service::taskFunction(void*) {
  std::array<uint8_t, Z21_MAX_PAYLOAD_SIZE> stack;
  sockaddr_in dest_addr_ip4;
  socklen_t socklen{sizeof(dest_addr_ip4)};

  /*
  // Multiple sockets, non-blocking ->
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(udp::sock_fd, &fds);

  select(FD_SETSIZE, &fds, NULL, NULL, NULL);

  if (FD_ISSET(udp::sock_fd, &fds)) {
    recvfrom...
  }
  */

  for (;;) {
    if (auto const len{recvfrom(udp::sock_fd,
                                data(stack),
                                size(stack) - 1,
                                0,
                                std::bit_cast<sockaddr*>(&dest_addr_ip4),
                                &socklen)};
        len < 0) {
      LOGE("recvfrom failed %s", strerror(errno));
      vTaskDelay(pdMS_TO_TICKS(task.timeout));
    } else if (len > 0) {
      std::lock_guard lock{_internal_mutex};
      receive({udp::sock_fd, std::bit_cast<sockaddr*>(&dest_addr_ip4), socklen},
              {data(stack), static_cast<size_t>(len)});
      execute();
    }
  }
}

/// \todo document
void Service::transmit(z21::Socket const& sock,
                       std::span<uint8_t const> datasets) {
  //
  if (_ws_sock_fds.contains(sock.fd)) {
    if (auto const err{httpd_queue_work(new http::Message{
          .sock_fd = sock.fd,
          .type = HTTPD_WS_TYPE_BINARY,
          .payload = {cbegin(datasets), cend(datasets)},
        })})
      LOGD("httpd_queue_work failed %s", esp_err_to_name(err));
  }
  //
  else if (sendto(sock.fd,
                  std::bit_cast<char*>(data(datasets)),
                  size(datasets),
                  0,
                  std::bit_cast<sockaddr*>(&sock.addr),
                  sock.len) < 0)
    LOGE("sendto failed %s", strerror(errno));
}

/// \todo document
bool Service::trackPower(bool on) {
  return trackPower(on, State::DCCOperations);
}

/// \todo document
bool Service::stop() {
  printf("%s\n", __PRETTY_FUNCTION__);
  return true;
}

/// \todo document
int32_t Service::serialNumber() const {
  return static_cast<int32_t>(little_endian_data2uint32(data(wifi::mac)));
}

/// \todo document
void Service::logoff(z21::Socket const& sock) {
  printf("%s\n", __PRETTY_FUNCTION__);
  /// \todo should this broadcast stop? if there are no more clients?
  if (empty(clients())) trackPower(false);
}

/// \todo document
z21::LocoInfo Service::locoInfo(uint16_t loco_addr) {
  return _dcc_service->locoInfo(loco_addr);
}

/// \todo document
void Service::locoDrive(uint16_t loco_addr,
                        z21::LocoInfo::SpeedSteps speed_steps,
                        uint8_t rvvvvvvv) {
  _dcc_service->locoDrive(loco_addr, speed_steps, rvvvvvvv);
}

/// \todo document
void Service::locoFunction(uint16_t loco_addr, uint32_t mask, uint32_t state) {
  _dcc_service->locoFunction(loco_addr, mask, state);
}

/// \todo document
z21::LocoInfo::Mode Service::locoMode(uint16_t loco_addr) {
  return _dcc_service->locoMode(loco_addr);
}

/// \todo document
void Service::locoMode(uint16_t loco_addr, z21::LocoInfo::Mode mode) {
  _dcc_service->locoMode(loco_addr, mode);
}

/// \todo document
z21::TurnoutInfo Service::turnoutInfo(uint16_t accy_addr) {
  LOGW("TODO IMPLEMENTED");
  return {};
}

/// \todo document
z21::AccessoryInfo Service::accessoryInfo(uint16_t accy_addr) {
  LOGW("TODO IMPLEMENTED");
  return {};
}

/// \todo document
void Service::turnout(uint16_t accy_addr, bool p, bool a, bool q) {
  LOGW("TODO IMPLEMENTED");
}

/// \todo document
void Service::accessory(uint16_t accy_addr, uint8_t dddddddd) {
  LOGW("TODO IMPLEMENTED");
}

/// \todo document
z21::TurnoutInfo::Mode Service::turnoutMode(uint16_t accy_addr) {
  LOGW("TODO IMPLEMENTED");
  return {};
}

/// \todo document
void Service::turnoutMode(uint16_t accy_addr, z21::TurnoutInfo::Mode mode) {
  LOGW("TODO IMPLEMENTED");
}

/// \todo document
bool Service::cvRead(uint16_t cv_addr) {
  return trackPower(true, State::DCCService) && _dcc_service->cvRead(cv_addr);
}

/// \todo document
bool Service::cvWrite(uint16_t cv_addr, uint8_t byte) {
  return trackPower(true, State::DCCService) &&
         _dcc_service->cvWrite(cv_addr, byte);
}

/// \todo document
void Service::cvPomRead(uint16_t loco_addr, uint16_t cv_addr) {
  _dcc_service->cvPomRead(loco_addr, cv_addr);
}

/// \todo document
void Service::cvPomWrite(uint16_t loco_addr, uint16_t cv_addr, uint8_t byte) {
  _dcc_service->cvPomWrite(loco_addr, cv_addr, byte);
}

/// \todo document
void Service::cvPomAccessoryRead(uint16_t accy_addr, uint16_t cv_addr) {
  _dcc_service->cvPomAccessoryRead(accy_addr, cv_addr);
}

/// \todo document
void Service::cvPomAccessoryWrite(uint16_t accy_addr,
                                  uint16_t cv_addr,
                                  uint8_t byte) {
  _dcc_service->cvPomAccessoryWrite(accy_addr, cv_addr, byte);
}

/// \todo document
z21::RailComData Service::railComData(uint16_t loco_addr) {
  return _dcc_service->railComData(loco_addr);
}

/// \todo document
z21::CommonSettings Service::commonSettings() { return {}; }

/// \todo document
void Service::commonSettings(z21::CommonSettings const& common_settings) {}

/// \todo document
z21::MmDccSettings Service::mmDccSettings() { return {}; }

/// \todo document
void Service::mmDccSettings(z21::MmDccSettings const& mm_dcc_settings) {}

/// \todo document
bool Service::trackPower(bool on, State dcc_state) {
  if (on) {
    switch (state.load()) {
      // Wait for suspend to complete
      case State::Suspend:
        while (state.load() != State::Suspended)
          vTaskDelay(pdMS_TO_TICKS(task.timeout));
        [[fallthrough]];

      // Clear errors
      case State::ShortCircuit:
        state.store(State::Suspended);
        led::bug(false);
        [[fallthrough]];

      //
      case State::Suspended:
        while (eTaskGetState(out::track::dcc::task.handle) != eSuspended)
          vTaskDelay(pdMS_TO_TICKS(task.timeout));
        if (auto expected{State::Suspended};
            state.compare_exchange_strong(expected, dcc_state))
          LOGI_TASK_RESUME(dcc::task);
        else assert(false);
        [[fallthrough]];

      // Already running
      case State::DCCOperations: [[fallthrough]];
      case State::DCCService: return true;

      //
      default:
        // broadcast track power off here?
        // or maybe programming?
        return false;
    }
  } else {

    /// \todo does... never... happen? Z21 app NEVER turn power off -.-
    auto expected{State::DCCOperations};
    state.compare_exchange_strong(expected, State::Suspend);
    expected = State::DCCService;
    state.compare_exchange_strong(expected, State::Suspend);

    return true;
  }
}

/// \todo document
// void Service::log(char const* str) { printf("%s\n", str); }

} // namespace z21
