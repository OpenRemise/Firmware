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

/// Task macros
///
/// \file   task.hpp
/// \author Vincent Hamp
/// \date   06/06/2025

#include <esp_task.h>
#include <array>

#pragma once

#define CONCAT_DETAIL(A, B) A##B
#define CONCAT(A, B) CONCAT_DETAIL(A, B)

/// Members common for \ref TASK and \ref SHARED_TASK
///
/// \param  NAME        Descriptive name
/// \param  PRIORITY    Priority
/// \param  CORE_ID     Core the task is pinned to
/// \param  TIMEOUT     Timeout in ms
#define COMMON_TASK_MEMBERS(NAME, PRIORITY, CORE_ID, TIMEOUT)                  \
  static constexpr char const* name{NAME};                                     \
  static constexpr UBaseType_t priority{PRIORITY};                             \
  static constexpr BaseType_t core_id{CORE_ID};                                \
  static constexpr TickType_t timeout{TIMEOUT};                                \
  static inline StaticTask_t tcb{};                                            \
  static inline TaskFunction_t function{NULL};                                 \
  static inline TaskHandle_t handle{NULL};                                     \
  static void create(TaskFunction_t f = function) {                            \
    handle = xTaskCreateStaticPinnedToCore(function = f,                       \
                                           name,                               \
                                           size(stack),                        \
                                           NULL,                               \
                                           priority,                           \
                                           data(stack),                        \
                                           &tcb,                               \
                                           core_id);                           \
  }                                                                            \
  static void destroy() {                                                      \
    vTaskDelete(handle);                                                       \
    handle = NULL;                                                             \
  }                                                                            \
  static void resume() { vTaskResume(handle); }                                \
  static void suspend() { vTaskSuspend(handle); }

/// Create task
///
/// \param  OBJECT      Instance name
/// \param  NAME        Descriptive name
/// \param  STACK_SIZE  Stack size in bytes
/// \param  PRIORITY    Priority
/// \param  CORE_ID     Core the task is pinned to
/// \param  TIMEOUT     Timeout in ms
#define TASK(OBJECT, NAME, STACK_SIZE, PRIORITY, CORE_ID, TIMEOUT)             \
  struct CONCAT(Task, __LINE__) {                                              \
    static inline std::array<StackType_t, STACK_SIZE> stack{};                 \
    COMMON_TASK_MEMBERS(NAME, PRIORITY, CORE_ID, TIMEOUT)                      \
  } OBJECT

/// Create shared task
///
/// \param  OBJECT      Instance name
/// \param  NAME        Descriptive name
/// \param  PRIORITY    Priority
/// \param  CORE_ID     Core the task is pinned to
/// \param  TIMEOUT     Timeout in ms
#define SHARED_TASK(OBJECT, NAME, PRIORITY, CORE_ID, TIMEOUT)                  \
  struct CONCAT(SharedTask, __LINE__) {                                        \
    COMMON_TASK_MEMBERS(NAME, PRIORITY, CORE_ID, TIMEOUT)                      \
  } OBJECT
