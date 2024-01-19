cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(CMakePrintHelpers)
include(FetchContent)

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY https://github.com/ZIMO-Elektronik/CMakeModules
  GIT_TAG v0.7.0)
FetchContent_MakeAvailable(CMakeModules)

set(EXTRA_COMPONENT_DIRS src)
if(IDF_TARGET STREQUAL linux)
  # Don't change COMPONENTS on ESP32* targets, it removes a shit ton of defaults
  list(APPEND COMPONENTS src tests)
  list(APPEND EXTRA_COMPONENT_DIRS tests $ENV{IDF_PATH}/tools/mocks/driver)
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Project version is taken from git describe
project(Firmware LANGUAGES ASM C CXX)

add_subdirectory(docs)

if(IDF_TARGET STREQUAL linux)
  include(CTest)
endif()

if(BUILD_TESTING AND IDF_TARGET STREQUAL linux)
  idf_build_get_property(TGT EXECUTABLE)
  gtest_discover_tests(${TGT})
endif()

file(
  DOWNLOAD
  https://raw.githubusercontent.com/ZIMO-Elektronik/.clang-format/master/.clang-format
  ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
file(GLOB_RECURSE SRC src/*.[ch]pp tests/*.[ch]pp)
add_clang_format_target(FirmwareFormat OPTIONS -i FILES ${SRC})
