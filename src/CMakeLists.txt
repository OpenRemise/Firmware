set(COMMON_SRC
    usb/dcc_ein/rx_task_function.cpp usb/dcc_ein/tx_task_function.cpp
    usb/rx_task_function.cpp utility.cpp)

if(IDF_TARGET STREQUAL linux)
  idf_component_register(
    SRCS
    ${COMMON_SRC}
    INCLUDE_DIRS
    .
    REQUIRES
    bblanchon__arduinojson
    driver
    esp_app_format
    nvs_flash
    EMBED_FILES
    ${CMAKE_BINARY_DIR}/http/ap/index.html)
else()
  idf_component_register(
    SRCS
    ${COMMON_SRC}
    analog/adc_task_function.cpp
    analog/convert.cpp
    analog/init.cpp
    analog/temp_task_function.cpp
    dcc/init.cpp
    dcc/loco.cpp
    dcc/operations_service.cpp
    dcc/service_service.cpp
    dcc/service.cpp
    http/ap/init.cpp
    http/ap/server.cpp
    http/sta/init.cpp
    http/sta/server.cpp
    http/init.cpp
    mdu/firmware_service.cpp
    mdu/init.cpp
    mdu/service.cpp
    mdu/zpp_service.cpp
    mem/nvs/accessories.cpp
    mem/nvs/base.cpp
    mem/nvs/init.cpp
    mem/nvs/locos.cpp
    mem/nvs/settings.cpp
    mem/spiffs/init.cpp
    mem/init.cpp
    ota/init.cpp
    ota/service.cpp
    out/track/dcc/resume.cpp
    out/track/dcc/rmt_dcc_encoder.c
    out/track/dcc/suspend.cpp
    out/track/dcc/task_function.cpp
    out/track/mdu/resume.cpp
    out/track/mdu/suspend.cpp
    out/track/mdu/task_function.cpp
    out/track/init.cpp
    out/track/resume.cpp
    out/track/suspend.cpp
    out/zusi/init.cpp
    out/zusi/resume.cpp
    out/zusi/suspend.cpp
    out/zusi/task_function.cpp
    out/zusi/zpp_load.cpp
    out/init.cpp
    out/resume.cpp
    out/suspend.cpp
    settings/init.cpp
    settings/service.cpp
    sys/init.cpp
    sys/service.cpp
    usb/susiv2/rx_task_function.cpp
    usb/susiv2/tx_task_function.cpp
    usb/init.cpp
    usb/rx_task_function.cpp
    usb/tx_task_function.cpp
    wifi/init.cpp
    zusi/init.cpp
    zusi/service.cpp
    app_main.cpp
    trace.cpp
    INCLUDE_DIRS
    .
    REQUIRES
    app_update
    bootloader_support
    driver
    esp_adc
    esp_app_format
    esp_http_server
    esp_wifi
    nvs_flash
    spiffs
    vfs
    EMBED_FILES
    ${CMAKE_BINARY_DIR}/http/ap/index.html)
endif()

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_23)

if(CONFIG_COMPILER_OPTIMIZATION_DEBUG)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE OPTIMIZATION="-Og")
elseif(CONFIG_COMPILER_OPTIMIZATION_SIZE)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE OPTIMIZATION="-Os")
elseif(CONFIG_COMPILER_OPTIMIZATION_PERF)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE OPTIMIZATION="-O2")
elseif(CONFIG_COMPILER_OPTIMIZATION_NONE)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE OPTIMIZATION="-O0")
else()
  message(FATAL_ERROR "Unknown optimization setting")
endif()

target_common_warnings(${COMPONENT_LIB} PRIVATE)

target_compile_options(
  ${COMPONENT_LIB}
  PUBLIC
    "$<$<COMPILE_LANGUAGE:CXX>:-include;${CMAKE_CURRENT_SOURCE_DIR}/config.hpp>"
    -Wfatal-errors)

# ESP-IDF stuff is full of -Werror=format...
idf_build_set_property(
  COMPILE_OPTIONS
  "-Wno-error=all;-Wno-error=extra;-Wno-error=implicit-function-declaration;-Wno-error=format"
  APPEND)

cpmaddpackage(
  NAME
  ZUSI
  GIT_REPOSITORY
  ssh://git@192.168.1.11:44933/ZIMO/ZUSI.git
  GIT_TAG
  master
  SYSTEM
  YES)
cpmaddpackage("gh:ZIMO-Elektronik/ULF_COM@0.0.1")
cpmaddpackage("gh:ZIMO-Elektronik/DCC#no_rmt")
cpmaddpackage("gh:ZIMO-Elektronik/DCC_EIN@0.2.0")
cpmaddpackage("gh:Neargye/magic_enum@0.9.5")
cpmaddpackage("gh:microsoft/GSL@4.0.0")

# fmt install option does not play nice with ESP-IDF
cpmaddpackage(
  NAME
  fmt
  GITHUB_REPOSITORY
  fmtlib/fmt
  GIT_TAG
  10.1.1
  SYSTEM
  YES
  OPTIONS
  "FMT_INSTALL OFF"
  "FMT_OS OFF")

target_link_libraries(
  ${COMPONENT_LIB} PUBLIC DCC_EIN::DCC_EIN fmt::fmt magic_enum::magic_enum
                          Microsoft.GSL::GSL ULF::COM ZUSI::ZUSI)

# Minify HTML shown by http::ap::Server
minify_html(http/ap/index.html ${CMAKE_BINARY_DIR}/http/ap/index.html)

# Create NVS partition image
if(NOT IDF_TARGET STREQUAL linux)
  nvs_create_partition_image(nvs ../nvs.csv FLASH_IN_PROJECT)

  # Assembly listening takes forever, add custom target
  add_custom_target(
    FirmwareLst
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers
            --wide ${CMAKE_PROJECT_NAME}.elf > ${CMAKE_PROJECT_NAME}.lst
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  add_dependencies(FirmwareLst app)
endif()
