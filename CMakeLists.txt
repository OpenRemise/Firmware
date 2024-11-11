cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(CMakePrintHelpers)
include(FetchContent)

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/CMakeModules"
  GIT_TAG v0.9.3)
FetchContent_MakeAvailable(CMakeModules)

set(EXTRA_COMPONENT_DIRS src)
if(IDF_TARGET STREQUAL linux)
  # Don't change COMPONENTS on ESP32* targets, it removes a shit ton of defaults
  list(APPEND COMPONENTS src tests)
  list(APPEND EXTRA_COMPONENT_DIRS tests $ENV{IDF_PATH}/tools/mocks/driver)
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake OPTIONAL)

# Project version is taken from git describe
project(Firmware LANGUAGES ASM C CXX)

# Build Frontend during config time
if(ESP_PLATFORM
   AND NOT IDF_TARGET STREQUAL linux
   AND NOT EXISTS ${Frontend_BINARY_DIR}/web)
  cpmaddpackage(
    NAME
    Frontend
    GITHUB_REPOSITORY
    "OpenRemise/Frontend"
    GIT_TAG
    develop
    DOWNLOAD_ONLY
    TRUE)

  execute_process(
    COMMAND ${CMAKE_COMMAND} -B${Frontend_BINARY_DIR} -G${CMAKE_GENERATOR}
    WORKING_DIRECTORY ${Frontend_SOURCE_DIR} COMMAND_ECHO STDOUT
                      COMMAND_ERROR_IS_FATAL ANY)
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${Frontend_BINARY_DIR} --target Frontend
            COMMAND_ECHO STDOUT COMMAND_ERROR_IS_FATAL ANY)

  # Create a list of absolute paths, relative paths and a flat representation
  # used by ESP-IDF embed
  file(GLOB_RECURSE SRC ${Frontend_BINARY_DIR}/web/*)
  set(REL_SRC ${SRC})
  list(TRANSFORM REL_SRC REPLACE ${Frontend_BINARY_DIR}/web/ "")
  set(FLAT_SRC ${REL_SRC})
  list(TRANSFORM FLAT_SRC REPLACE "[,-\/]" "_")

  # Flatten web directory to embed
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                          ${Frontend_BINARY_DIR}/embed)
  foreach(FILE IN ZIP_LISTS SRC FLAT_SRC)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${FILE_0}
                            ${Frontend_BINARY_DIR}/embed/${FILE_1})
    target_add_binary_data(${PROJECT_NAME}.elf
                           ${Frontend_BINARY_DIR}/embed/${FILE_1} BINARY)
  endforeach()

  # Create a header file which maps actual file names to embeded binaries
  set(HEADER ${CMAKE_BINARY_DIR}/frontend_embeds.hpp)
  file(WRITE ${HEADER} "#pragma once\n" "#include <array>\n")
  file(APPEND ${HEADER} "extern \"C\" {\n")
  foreach(FILE ${FLAT_SRC})
    file(APPEND ${HEADER} "extern char const _binary_${FILE}_start;\n"
                          "extern char const _binary_${FILE}_end;\n")
  endforeach()
  file(APPEND ${HEADER} "}\n")
  file(APPEND ${HEADER} "inline constexpr std::array frontend_embeds{\n")
  foreach(FILE IN ZIP_LISTS REL_SRC FLAT_SRC)
    file(
      APPEND ${HEADER}
      "std::array{\"${FILE_0}\", &_binary_${FILE_1}_start, &_binary_${FILE_1}_end},\n"
    )
  endforeach()
  file(APPEND ${HEADER} "};\n")
endif()

add_subdirectory(docs)

file(DOWNLOAD
     "https://github.com/OpenRemise/.github/raw/master/data/.clang-format"
     ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
file(GLOB_RECURSE SRC src/*.[ch]pp tests/*.[ch]pp)
add_clang_format_target(FirmwareFormat OPTIONS -i FILES ${SRC})
