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
/// \file   mem/nvs/base.hpp
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <nvs.h>
#include <cstdint>
#include <span>
#include <string_view>

namespace mem::nvs {

/// NVS base
///
/// nvs::Base is a convenience wrapper over the NVS C-API and forms the basis of
/// all NVS classes. The [RAII](https://en.cppreference.com/w/cpp/language/raii)
/// idiom ensures that the desired namespace is opened in the constructor and
/// that any write operations are committed in the destructor and the namespace
/// is closed again.
///
/// A nested iterator type (nvs::Base::Iterator) ensures that the keys of the
/// namespace can be iterated over.
class Base {
public:
  /// Sentinel type for Iterator
  struct Sentinel {};

  /// Wrapper around C-style [NVS
  /// iterators](https://docs.espressif.com/projects/esp-idf/en/\idf_ver/esp32s3/api-reference/storage/nvs_flash.html#nvs-iterators)
  struct Iterator {
    using value_type = nvs_entry_info_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;

    /// Ctor
    ///
    /// Creates an iterator to enumerate NVS entries.
    ///
    /// \param namespace_name Namespace to iterate over
    Iterator(char const* namespace_name)
      : _err{nvs_entry_find(
          NVS_DEFAULT_PART_NAME, namespace_name, NVS_TYPE_ANY, &_it)} {}

    /// Dtor
    ///
    /// Release iterator.
    ~Iterator() { nvs_release_iterator(_it); }

    reference operator*() const {
      nvs_entry_info_t info;
      nvs_entry_info(_it, &info);
      return info;
    }

    Iterator& operator++() {
      _err = nvs_entry_next(&_it);
      return *this;
    }

    void operator++(int) { this->operator++(); }

    /// Equality comparison
    ///
    /// esp_err_t implicitly converts to bool. Any other value than 'ESP_OK'
    /// indicates that we have reached the end.
    ///
    /// \return Return value of last call to nvs_entry_next
    friend constexpr bool operator==(Iterator const& lhs, Sentinel) {
      return lhs._err;
    }

  private:
    nvs_iterator_t _it{};
    esp_err_t _err{};
  };

  static_assert(std::input_iterator<Iterator>);

  auto begin() const { return Iterator{_namespace_name}; }
  auto cbegin() const { return begin(); }
  auto end() const { return Sentinel{}; }
  auto cend() const { return end(); }

  esp_err_t find(std::string const& key) const;
  esp_err_t erase(std::string const& key);
  esp_err_t eraseAll();

protected:
  Base(char const* namespace_name, nvs_open_mode_t open_mode);
  ~Base();

  std::string getBlob(std::string const& key) const;
  esp_err_t setBlob(std::string const& key, std::string_view str);

  uint8_t getU8(std::string const& key) const;
  esp_err_t setU8(std::string const& key, uint8_t value);

  uint8_t getU16(std::string const& key) const;
  esp_err_t setU16(std::string const& key, uint16_t value);

private:
  // The getters and setters in this class rely on SSO inside the std::string
  // class. The default capacity of std::string must be large enough to hold a
  // string of length NVS_KEY_NAME_MAX_SIZE.
  //
  // The memory layout of std::string looks something like this
  // char*
  // size_t
  // char[15+1]
  static_assert(sizeof(std::string) == sizeof(std::string::pointer) +
                                         sizeof(std::string::size_type) +
                                         (15uz + 1uz));
  static_assert(NVS_KEY_NAME_MAX_SIZE <= 15uz + 1uz);

  /// Store namespace name (mainly for iterator)
  char const* _namespace_name{};

  /// Opaque pointer type representing non-volatile storage handle
  nvs_handle_t _handle{};

  /// Flag to indicate commit pending
  bool _commit_pending{};
};

} // namespace mem::nvs