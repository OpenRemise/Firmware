cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(CMakePrintHelpers)
include(FetchContent)

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/CMakeModules"
  GIT_TAG v0.9.5)
FetchContent_MakeAvailable(CMakeModules)

set(EXTRA_COMPONENT_DIRS src)
if(IDF_TARGET STREQUAL linux)
  # Don't change COMPONENTS on ESP32* targets, it removes a shit ton of defaults
  list(APPEND COMPONENTS src tests)
  list(APPEND EXTRA_COMPONENT_DIRS tests $ENV{IDF_PATH}/tools/mocks/driver)
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

version_from_git()
project(
  Firmware
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES ASM C CXX)

# Custom release target to create .zip
if(ESP_PLATFORM
   AND NOT IDF_TARGET STREQUAL linux
   AND CONFIG_COMPILER_OPTIMIZATION_SIZE)
  add_custom_target(
    FirmwareRelease
    COMMAND
      ${CMAKE_COMMAND} -E tar "cf" ${PROJECT_NAME}-DRV8328-Test.zip
      --format=zip #
      flasher_args.json #
      bootloader/bootloader.bin #
      Firmware.bin #
      partition_table/partition-table.bin #
      ota_data_initial.bin #
      nvs.bin #
    DEPENDS all
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

file(DOWNLOAD
     "https://github.com/OpenRemise/.github/raw/master/data/.clang-format"
     ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
file(GLOB_RECURSE SRC src/*.[ch]pp tests/*.[ch]pp)
add_clang_format_target(FirmwareFormat OPTIONS -i FILES ${SRC})
