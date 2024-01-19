#pragma once

#include <ArduinoJson.h>
#include <dcc/dcc.hpp>
#include <map>
#include <mutex>
#include <optional>
#include "http/request.hpp"
#include "http/response.hpp"

namespace dcc {

class ServiceService {
public:
  http::Response serviceGetRequest(http::Request const& req);
  http::Response servicePutRequest(http::Request const& req);

protected:
  void loop();

private:
  using key_type = uint16_t;
  using mapped_type = std::optional<uint8_t>;
  using value_type = std::pair<key_type const, mapped_type>;

  void execute();
  void clear();

  void sendCvPackets();
  void sendCvWritePackets(key_type addr, mapped_type value);
  void sendCvReadPackets(key_type addr);
  void sendResetPacket() const;

  void sendPacketNTimes(dcc::Packet const& packet, size_t n) const;
  void optionalDelayForAddress(key_type addr) const;

  std::pair<std::optional<key_type>, std::optional<mapped_type>>
  jsonPair2keyValue(JsonPair const& p) const;
  std::optional<key_type> jsonString2key(JsonString const& str) const;
  std::optional<mapped_type> jsonVariant2value(JsonVariantConst const& v) const;

  value_type getTodoThenErase();
  std::optional<bool> receiveBit() const;
  std::optional<uint8_t> receiveByte() const;

  static constexpr auto N_{5uz};

  /*
  Ich weiß noch nicht ob die Aufteilung so Sinn macht? Mein ursprünglicher
  Gedanke war in _todo alles zu haben was noch getan wern muss und in _done was
  bereits gelesen/geschrieben wurde. Wenn also neue Befehle kommen würde ich
  - wenns ein read is schaun obs in _done scho drin is
  - wenns ein write is schaun obs in _done is und ob da Wert ident is
  Das Problem an dem Ansatz is aber, dass ma ned "force" neu lesen/schreiben
  kann...
  */
  std::map<key_type, mapped_type> _todo;
  std::map<key_type, mapped_type> _done;
  std::mutex _internal_mutex;
};

}  // namespace dcc