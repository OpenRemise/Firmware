/// Initialize SPIFFS
///
/// \file   mem/spiffs/init.cpp
/// \author Vincent Hamp
/// \date   10/02/2023

#include "init.hpp"
#include <esp_spiffs.h>

namespace mem::spiffs {

///
esp_err_t init() {
  static constexpr esp_vfs_spiffs_conf_t conf{.base_path = "",
                                              .partition_label = NULL,
                                              .max_files = 5,
                                              .format_if_mount_failed = true};

  // Use settings defined above to initialize and mount SPIFFS filesystem.
  // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
  return esp_vfs_spiffs_register(&conf);
}

}  // namespace mem::spiffs