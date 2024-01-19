/// Log macros without TAG parameter
///
/// \file   log.h
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_log.h>

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define PREFIX_LOG(LOG_MACRO, ...)                                             \
  do {                                                                         \
    ESP_##LOG_MACRO(__FILE__ ":" STRINGIZE(__LINE__), __VA_ARGS__);            \
  } while (0)

#define LOG_BUFFER_HEX_LEVEL(...) PREFIX_LOG(LOG_BUFFER_HEX_LEVEL, __VA_ARGS__)
#define LOG_BUFFER_CHAR_LEVEL(...)                                             \
  PREFIX_LOG(LOG_BUFFER_CHAR_LEVEL, __VA_ARGS__)
#define LOG_BUFFER_HEXDUMP(...) PREFIX_LOG(LOG_BUFFER_HEXDUMP, __VA_ARGS__)
#define LOG_BUFFER_HEX(...) PREFIX_LOG(LOG_BUFFER_HEX, __VA_ARGS__)
#define LOG_BUFFER_CHAR(...) PREFIX_LOG(LOG_BUFFER_CHAR, __VA_ARGS__)
#define EARLY_LOGE(...) PREFIX_LOG(EARLY_LOGE, __VA_ARGS__)
#define EARLY_LOGW(...) PREFIX_LOG(EARLY_LOGW, __VA_ARGS__)
#define EARLY_LOGI(...) PREFIX_LOG(EARLY_LOGI, __VA_ARGS__)
#define EARLY_LOGD(...) PREFIX_LOG(EARLY_LOGD, __VA_ARGS__)
#define EARLY_LOGV(...) PREFIX_LOG(EARLY_LOGV, __VA_ARGS__)
#define LOGE(...) PREFIX_LOG(LOGE, __VA_ARGS__)
#define LOGW(...) PREFIX_LOG(LOGW, __VA_ARGS__)
#define LOGI(...) PREFIX_LOG(LOGI, __VA_ARGS__)
#define LOGD(...) PREFIX_LOG(LOGD, __VA_ARGS__)
#define LOGV(...) PREFIX_LOG(LOGV, __VA_ARGS__)
#define DRAM_LOGE(...) PREFIX_LOG(DRAM_LOGE, __VA_ARGS__)
#define DRAM_LOGW(...) PREFIX_LOG(DRAM_LOGW, __VA_ARGS__)
#define DRAM_LOGI(...) PREFIX_LOG(DRAM_LOGI, __VA_ARGS__)
#define DRAM_LOGD(...) PREFIX_LOG(DRAM_LOGD, __VA_ARGS__)
#define DRAM_LOGV(...) PREFIX_LOG(DRAM_LOGV, __VA_ARGS__)

#define LOGI_TASK_RESUME(task_handle)                                          \
  do {                                                                         \
    LOGI("Resume %s task", pcTaskGetName(task_handle));                        \
    vTaskResume(task_handle);                                                  \
  } while (0)

#define LOGI_TASK_SUSPEND(task_handle)                                         \
  do {                                                                         \
    LOGI("Suspend %s task", pcTaskGetName(task_handle));                       \
    vTaskSuspend(task_handle);                                                 \
  } while (0)
