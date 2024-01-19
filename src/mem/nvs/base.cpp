/// NVS base
///
/// \file   mem/nvs/base.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "base.hpp"
#include <cstring>

namespace mem::nvs {

/// TODO
Base::Base(char const* namespace_name, nvs_open_mode_t open_mode)
  : _namespace_name{namespace_name} {
  assert(namespace_name);
  ESP_ERROR_CHECK(nvs_open(namespace_name, open_mode, &_handle));
}

/// TODO
Base::~Base() {
  if (_commit_pending) ESP_ERROR_CHECK(nvs_commit(_handle));
  nvs_close(_handle);
}

/// TODO
///
/// std::string enforces null-terminated string
esp_err_t Base::erase(std::string const& key) {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  auto const err{nvs_erase_key(_handle, key.c_str())};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

/// TODO
esp_err_t Base::eraseAll() {
  auto const err{nvs_erase_all(_handle)};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

/// TODO
std::string Base::getBlob(std::string const& key) const {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  size_t len;
  if (nvs_get_blob(_handle, key.c_str(), NULL, &len) != ESP_OK) return {};
  std::string blob(len, '\0');
  if (nvs_get_blob(_handle, key.c_str(), data(blob), &len) != ESP_OK) return {};
  return blob;
}

/// TODO
esp_err_t Base::setBlob(std::string const& key, std::string_view str) {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  auto const err{nvs_set_blob(_handle, key.c_str(), data(str), size(str))};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

/// TODO
uint8_t Base::getU8(std::string const& key) const {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  uint8_t value;
  if (nvs_get_u8(_handle, key.c_str(), &value) != ESP_OK) return {};
  return value;
}

/// TODO
esp_err_t Base::setU8(std::string const& key, uint8_t value) {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  auto const err{nvs_set_u8(_handle, key.c_str(), value)};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

/// TODO
uint8_t Base::getU16(std::string const& key) const {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  uint16_t value;
  if (nvs_get_u16(_handle, key.c_str(), &value) != ESP_OK) return {};
  return value;
}

/// TODO
esp_err_t Base::setU16(std::string const& key, uint16_t value) {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  auto const err{nvs_set_u16(_handle, key.c_str(), value)};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

}  // namespace mem::nvs