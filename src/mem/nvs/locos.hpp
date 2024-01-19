/// NVS "locos" namespace
///
/// \file   mem/nvs/locos.hpp
/// \author Vincent Hamp
/// \date   17/02/2023

#pragma once

#include <dcc/dcc.hpp>
#include "base.hpp"
#include "dcc/loco.hpp"

namespace mem::nvs {

class Locos : public Base {
public:
  explicit Locos() : Base{"locos", NVS_READWRITE} {}

  dcc::NvLocoBase get(dcc::Address::value_type addr) const;
  dcc::NvLocoBase get(std::string const& key) const;
  esp_err_t set(dcc::Address::value_type addr, dcc::NvLocoBase const& loco);
  esp_err_t set(std::string const& key, dcc::NvLocoBase const& loco);
  esp_err_t erase(dcc::Address::value_type addr);

  std::string address2key(dcc::Address::value_type addr) const;
  dcc::Address::value_type key2address(std::string_view key) const;
};

}  // namespace mem::nvs