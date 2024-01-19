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

class Base {
public:
  ///
  struct Sentinel {};

  ///
  struct Iterator {
    using value_type = nvs_entry_info_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;

    ///
    Iterator(char const* namespace_name)
      : err_{nvs_entry_find(
          NVS_DEFAULT_PART_NAME, namespace_name, NVS_TYPE_ANY, &it_)} {}

    ///
    ~Iterator() { nvs_release_iterator(it_); }

    ///
    reference operator*() const {
      nvs_entry_info_t info;
      nvs_entry_info(it_, &info);
      return info;
    }

    ///
    Iterator& operator++() {
      err_ = nvs_entry_next(&it_);
      return *this;
    }

    /// (void)r++
    void operator++(int) { this->operator++(); }

    ///
    friend constexpr bool operator==(Iterator const& lhs, Sentinel) {
      return lhs.err_;
    }

  private:
    nvs_iterator_t it_{};
    esp_err_t err_{};
  };

#if __GNUC__ > 11 || (__GNUC__ == 11 && __GNUC_MINOR__ > 2)
  static_assert(std::input_iterator<Iterator>);
#endif

  auto begin() const { return Iterator{_namespace_name}; }
  auto cbegin() const { return begin(); }
  auto end() const { return Sentinel{}; }
  auto cend() const { return end(); }

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
  char const* _namespace_name{};
  nvs_handle_t _handle{};
  bool _commit_pending{};
};

}  // namespace mem::nvs