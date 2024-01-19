#pragma once

#include <map>
#include <mutex>
#include <optional>
#include "http/request.hpp"
#include "http/response.hpp"
#include "loco.hpp"

namespace dcc {

class OperationsService {
public:
  explicit OperationsService();

  http::Response locosGetRequest(http::Request const& req);

  // Well, yeah, delete shit from list and nvs, check that the deleted one ain't
  // one where we currently store an iterator to
  http::Response locosDeleteRequest(http::Request const& req);

  // In case changes get pushed by a put request push function and speed
  // commands to the front buffer. (maybe... twice?)
  http::Response locosPutRequest(http::Request const& req);

protected:
  void loop();

private:
  using Locos = std::map<Address::value_type, Loco>;

  void execute();
  void sendDecoderPackets();
  void sendIdlePacket() const;

  Locos _locos{};
  std::optional<Locos::iterator> _it{};
  std::mutex _internal_mutex;
};

}  // namespace dcc