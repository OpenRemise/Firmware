set(COMMON_SRC
    dcc/loco.cpp out/track/current_limit.cpp usb/dcc_ein/task_function.cpp
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
    esp_http_server
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
    dcc/service.cpp
    http/ap/init.cpp
    http/ap/server.cpp
    http/sta/init.cpp
    http/sta/server.cpp
    http/init.cpp
    mdu/init.cpp
    mdu/service.cpp
    mdu/zpp_service.cpp
    mdu/zsu_service.cpp
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
    out/track/dcc/suspend.cpp
    out/track/dcc/task_function.cpp
    out/track/decup/resume.cpp
    out/track/decup/rmt_decup_encoder.c
    out/track/decup/suspend.cpp
    out/track/decup/task_function.cpp
    out/track/mdu/resume.cpp
    out/track/mdu/suspend.cpp
    out/track/mdu/task_function.cpp
    out/track/init.cpp
    out/zusi/init.cpp
    out/zusi/resume.cpp
    out/zusi/suspend.cpp
    out/zusi/task_function.cpp
    out/init.cpp
    out/suspend.cpp
    udp/init.cpp
    usb/decup_ein/task_function.cpp
    usb/susiv2/task_function.cpp
    usb/init.cpp
    usb/rx_task_function.cpp
    usb/tx_task_function.cpp
    wifi/init.cpp
    z21/init.cpp
    z21/service.cpp
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

target_common_warnings(${COMPONENT_LIB} PRIVATE)

target_compile_options(
  ${COMPONENT_LIB}
  PUBLIC
    "$<$<COMPILE_LANGUAGE:CXX>:-include;${CMAKE_CURRENT_SOURCE_DIR}/config.hpp>"
    -Wfatal-errors)

# ESP-IDF stuff is full of -Werror=format...
idf_build_set_property(
  COMPILE_OPTIONS
  "-Wno-error=all;-Wno-error=extra;-Wno-error=implicit-function-declaration;-Wno-error=format;-Wno-error=switch"
  APPEND)

cpmaddpackage("gh:ZIMO-Elektronik/DECUP#develop")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_COM@0.0.2")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_DCC_EIN@0.3.1")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_DECUP_EIN#develop")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_MDU_EIN#master")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_SUSIV2#master")
cpmaddpackage("gh:ZIMO-Elektronik/ZUSI@0.7.0")
cpmaddpackage("gh:microsoft/GSL@4.1.0")
cpmaddpackage("gh:Neargye/magic_enum@0.9.6")

# According to this answer here https://stackoverflow.com/a/55312360/5840652
# magic enum works up to INT16_MAX
math(EXPR INT16_MAX "(1 << 15) - 1" OUTPUT_FORMAT DECIMAL)
target_compile_definitions(
  magic_enum INTERFACE MAGIC_ENUM_RANGE_MIN=0 MAGIC_ENUM_RANGE_MAX=${INT16_MAX})

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
  ${COMPONENT_LIB}
  PUBLIC DECUP::DECUP
         fmt::fmt
         magic_enum::magic_enum
         Microsoft.GSL::GSL
         ULF::COM
         ULF::DCC_EIN
         ULF::DECUP_EIN
         ULF::MDU_EIN
         ULF::SUSIV2
         Z21::Z21
         ZUSI::ZUSI)

# Minify HTML shown by http::ap::Server
minify_html(http/ap/index.html ${CMAKE_BINARY_DIR}/http/ap/index.html)

# Create NVS and spiffs partition images
if(NOT IDF_TARGET STREQUAL linux)
  cpmaddpackage("gh:OpenRemise/Frontend#master")
  spiffs_create_partition_image(storage ${Frontend_BINARY_DIR}/build/web
                                FLASH_IN_PROJECT DEPENDS Frontend)

  nvs_create_partition_image(nvs ../nvs.csv FLASH_IN_PROJECT)

  # Assembly listening takes forever, add custom target
  add_custom_target(
    FirmwareLst
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers
            --wide ${CMAKE_PROJECT_NAME}.elf > ${CMAKE_PROJECT_NAME}.lst
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  add_dependencies(FirmwareLst app)
endif()
