cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(CMakePrintHelpers)
include(FetchContent)

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/CMakeModules"
  GIT_TAG v0.9.5)
FetchContent_MakeAvailable(CMakeModules)

set(OPENREMISE_FRONTEND_SOURCE_DIR
    ""
    CACHE STRING "Overrides the frontend source code directory")

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

# Frontend
if(ESP_PLATFORM
   AND NOT IDF_TARGET STREQUAL linux
   AND NOT EXISTS ${Frontend_BINARY_DIR})

  # Build from local repo or devel branch at config time
  if(NOT ${OPENREMISE_FRONTEND_SOURCE_DIR} STREQUAL "" OR CMAKE_BUILD_TYPE
                                                          STREQUAL Debug)
    if(OPENREMISE_FRONTEND_SOURCE_DIR)
      cpmaddpackage(NAME Frontend SOURCE_DIR ${OPENREMISE_FRONTEND_SOURCE_DIR}
                    DOWNLOAD_ONLY TRUE)
    else()
      cpmaddpackage(
        NAME
        Frontend
        GITHUB_REPOSITORY
        "OpenRemise/Frontend"
        GIT_TAG
        turnouts
        DOWNLOAD_ONLY
        TRUE)
    endif()

    execute_process(
      COMMAND ${CMAKE_COMMAND} -B${Frontend_BINARY_DIR}
      WORKING_DIRECTORY ${Frontend_SOURCE_DIR} COMMAND_ECHO STDOUT
                        COMMAND_ERROR_IS_FATAL ANY)
    execute_process(
      COMMAND ${CMAKE_COMMAND} --build ${Frontend_BINARY_DIR} --target Frontend
              COMMAND_ECHO STDOUT COMMAND_ERROR_IS_FATAL ANY)
    set(FRONTEND_WEB_DIR ${Frontend_BINARY_DIR}/web)

    # ... or use pre-built official release
  elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    cpmaddpackage(
      NAME
      Frontend
      URL
      "https://github.com/OpenRemise/Frontend/releases/download/v${VERSION_FROM_GIT}/Frontend-${VERSION_FROM_GIT}.zip"
      VERSION
      ${OPENREMISE_FRONTENVERSION_FROM_GITD_VERSION}
      DOWNLOAD_ONLY
      ON)
    set(FRONTEND_WEB_DIR ${Frontend_SOURCE_DIR})
  endif()

  if(NOT EXISTS ${FRONTEND_WEB_DIR})
    message(FATAL_ERROR "Failed to build frontend")
  endif()

  # Create a list of absolute paths, relative paths and a flat representation
  # used by ESP-IDF embed
  file(GLOB_RECURSE SRC ${FRONTEND_WEB_DIR}/*)
  set(REL_SRC ${SRC})
  list(TRANSFORM REL_SRC REPLACE ${FRONTEND_WEB_DIR}/ "")
  set(FLAT_SRC ${REL_SRC})
  list(TRANSFORM FLAT_SRC REPLACE "[,-\/]" "_")

  # Flatten and gzip web directory to embed
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                          ${Frontend_BINARY_DIR}/embed)
  foreach(FILE IN ZIP_LISTS SRC FLAT_SRC)
    file(
      ARCHIVE_CREATE
      OUTPUT
      ${Frontend_BINARY_DIR}/embed/${FILE_1}_gz
      PATHS
      ${FILE_0}
      FORMAT
      raw
      COMPRESSION
      GZip
      VERBOSE)
    target_add_binary_data(${PROJECT_NAME}.elf
                           ${Frontend_BINARY_DIR}/embed/${FILE_1}_gz BINARY)
  endforeach()

  # Create a header file which maps actual file names to embedded binaries
  set(HEADER ${CMAKE_BINARY_DIR}/frontend_embeds.hpp)
  file(WRITE ${HEADER} "#pragma once\n" "#include <array>\n")
  file(APPEND ${HEADER} "extern \"C\" {\n")
  foreach(FILE ${FLAT_SRC})
    file(APPEND ${HEADER} "extern char const _binary_${FILE}_gz_start;\n"
                          "extern char const _binary_${FILE}_gz_end;\n")
  endforeach()
  file(APPEND ${HEADER} "}\n")
  file(APPEND ${HEADER} "inline constexpr std::array frontend_embeds{\n")
  foreach(FILE IN ZIP_LISTS REL_SRC FLAT_SRC)
    file(
      APPEND ${HEADER}
      "std::array{\"${FILE_0}\", &_binary_${FILE_1}_gz_start, &_binary_${FILE_1}_gz_end},\n"
    )
  endforeach()
  file(APPEND ${HEADER} "};\n")
endif()

# Custom release target to create .zip
if(ESP_PLATFORM
   AND NOT IDF_TARGET STREQUAL linux
   AND CONFIG_COMPILER_OPTIMIZATION_SIZE)
  add_custom_target(
    FirmwareRelease
    COMMAND
      ${CMAKE_COMMAND} -E tar "cf" ${PROJECT_NAME}-${PROJECT_VERSION}.zip
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

add_subdirectory(docs)

file(DOWNLOAD
     "https://github.com/OpenRemise/.github/raw/master/data/.clang-format"
     ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
file(GLOB_RECURSE SRC src/*.[ch]pp tests/*.[ch]pp)
add_clang_format_target(FirmwareFormat OPTIONS -i FILES ${SRC})
