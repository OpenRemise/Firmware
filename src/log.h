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

/// Log macros
///
/// \file   log.h
/// \author Vincent Hamp
/// \date   09/02/2023

#pragma once

#include <esp_log.h>
#include <boost/preprocessor.hpp>

#define PREFIX_LOG(LOG_MACRO, ...)                                             \
  do {                                                                         \
    ESP_##LOG_MACRO(__FILE__ ":" BOOST_PP_STRINGIZE(__LINE__), __VA_ARGS__);   \
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

/// Log task creation
#define LOGI_TASK_CREATE(TASK)                                                 \
  do {                                                                         \
    LOGI("Create %s task", TASK.name);                                         \
    TASK.create();                                                             \
  } while (0)

/// Helper to fit
/// https://live.boost.org/doc/libs/1_88_0/libs/preprocessor/doc/ref/seq_for_each.html
#define BOOST_PP_LOGI_TASK_CREATE(R, DATA, ELEM) LOGI_TASK_CREATE(ELEM);

/// Log tasks creation
#define LOGI_TASKS_CREATE(...)                                                 \
  BOOST_PP_SEQ_FOR_EACH(                                                       \
    BOOST_PP_LOGI_TASK_CREATE, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

/// Log task destruction implementation for NULL
#define LOGI_TASK_DESTROY_0()                                                  \
  do {                                                                         \
    LOGI("Destroy %s task", pcTaskGetName(NULL));                              \
    vTaskDelete(NULL);                                                         \
    std::unreachable();                                                        \
  } while (0)

/// Log task destruction implementation for task handle
#define LOGI_TASK_DESTROY_1(TASK)                                              \
  do {                                                                         \
    LOGI("Destroy %s task", TASK.name);                                        \
    TASK.destroy();                                                            \
  } while (0)

/// Log task destruction
#define LOGI_TASK_DESTROY(...)                                                 \
  BOOST_PP_OVERLOAD(LOGI_TASK_DESTROY_, __VA_ARGS__)(__VA_ARGS__)

/// Log task resumption
#define LOGI_TASK_RESUME(TASK)                                                 \
  do {                                                                         \
    LOGI("Resume %s task", TASK.name);                                         \
    TASK.resume();                                                             \
  } while (0)

/// Log task suspension implementation for NULL
#define LOGI_TASK_SUSPEND_0()                                                  \
  do {                                                                         \
    LOGI("Suspend %s task", pcTaskGetName(NULL));                              \
    vTaskSuspend(NULL);                                                        \
  } while (0)

/// Log task suspension implementation for task handle
#define LOGI_TASK_SUSPEND_1(TASK)                                              \
  do {                                                                         \
    LOGI("Suspend %s task", TASK.name);                                        \
    TASK.suspend();                                                            \
  } while (0)

/// Log task suspension
#define LOGI_TASK_SUSPEND(...)                                                 \
  BOOST_PP_OVERLOAD(LOGI_TASK_SUSPEND_, __VA_ARGS__)(__VA_ARGS__)
