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

/// NVS base
///
/// \file   mem/nvs/base.cpp
/// \author Vincent Hamp
/// \date   09/02/2023

#include "base.hpp"
#include <cstring>

namespace mem::nvs {

/// Ctor
///
/// Open non-volatile storage with a given namespace from the default NVS
/// partition.
///
/// \param  namespace_name  Namespace to open
/// \param  open_mode       Mode of opening the non-volatile storage
Base::Base(char const* namespace_name, nvs_open_mode_t open_mode)
  : _namespace_name{namespace_name} {
  assert(namespace_name);
  ESP_ERROR_CHECK(nvs_open(namespace_name, open_mode, &_handle));
}

/// Dtor
///
/// Write any pending changes to non-volatile storage, then close the storage
/// handle and free any allocated resources.
Base::~Base() {
  if (_commit_pending) ESP_ERROR_CHECK(nvs_commit(_handle));
  nvs_close(_handle);
}

/// Lookup key-value pair with given key name
///
/// \param  key                   Key name
/// \retval ESP_OK                Key found
/// \retval ESP_ERR_NVS_NOT_FOUND Key not found
/// \retval ESP_FAIL              Internal error
esp_err_t Base::find(std::string const& key) const {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  return nvs_find_key(_handle, key.c_str(), NULL);
}

/// Erase key-value pair with given key name
///
/// \param  key                   Key name
/// \retval ESP_OK                Erase operation was successful
/// \retval ESP_FAIL              Internal error
/// \retval ESP_ERR_NVS_NOT_FOUND Requested key doesn't exist
esp_err_t Base::erase(std::string const& key) {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  auto const err{nvs_erase_key(_handle, key.c_str())};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

/// Erase all key-value pairs in a namespace
///
/// \retval ESP_OK    Erase operation was successful
/// \retval ESP_FAIL  Internal error
esp_err_t Base::eraseAll() {
  auto const err{nvs_erase_all(_handle)};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

/// Get blob value for given key
///
/// \param  key Key name
/// \return Blob value as `std::string`
std::string Base::getBlob(std::string const& key) const {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  size_t len;
  if (nvs_get_blob(_handle, key.c_str(), NULL, &len) != ESP_OK) return {};
  std::string blob(len, '\0');
  if (nvs_get_blob(_handle, key.c_str(), data(blob), &len) != ESP_OK) return {};
  return blob;
}

/// Set blob value for given key
///
/// \param  key                           Key name
/// \param  str                           Blob value
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
/// \retval ESP_ERR_NVS_VALUE_TOO_LONG    String value is too long
esp_err_t Base::setBlob(std::string const& key, std::string_view str) {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  auto const err{nvs_set_blob(_handle, key.c_str(), data(str), size(str))};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

/// Get uint8_t value for given key
///
/// \param  key Key name
/// \return uint8_t value
uint8_t Base::getU8(std::string const& key) const {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  uint8_t value;
  if (nvs_get_u8(_handle, key.c_str(), &value) != ESP_OK) return {};
  return value;
}

/// Set uint8_t value for given key
///
/// \param  key                           Key name
/// \param  value                         uint8_t value
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
esp_err_t Base::setU8(std::string const& key, uint8_t value) {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  auto const err{nvs_set_u8(_handle, key.c_str(), value)};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

/// Get uint16_t value for given key
///
/// \param  key Key name
/// \return uint16_t value
uint8_t Base::getU16(std::string const& key) const {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  uint16_t value;
  if (nvs_get_u16(_handle, key.c_str(), &value) != ESP_OK) return {};
  return value;
}

/// Set uint16_t value for given key
///
/// \param  key                           Key name
/// \param  value                         uint16_t value
/// \retval ESP_OK                        Value was set successfully
/// \retval ESP_FAIL                      Internal error
/// \retval ESP_ERR_NVS_INVALID_NAME      Key name doesn't satisfy constraints
/// \retval ESP_ERR_NVS_NOT_ENOUGH_SPACE  Not enough space
/// \retval ESP_ERR_NVS_REMOVE_FAILED     Value wasn't updated because flash
///                                       write operation has failed
esp_err_t Base::setU16(std::string const& key, uint16_t value) {
  assert(size(key) < NVS_KEY_NAME_MAX_SIZE);
  auto const err{nvs_set_u16(_handle, key.c_str(), value)};
  if (err == ESP_OK) _commit_pending = true;
  return err;
}

} // namespace mem::nvs