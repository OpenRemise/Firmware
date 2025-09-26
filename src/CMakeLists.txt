set(COMMON_SRC
    drv/out/track/current_limit.cpp
    intf/usb/rx_task_function.cpp
    mem/nvs/accessories.cpp
    mem/nvs/base.cpp
    mem/nvs/init.cpp
    mem/nvs/locos.cpp
    mem/nvs/settings.cpp
    mem/nvs/turnouts.cpp
    mem/nvs/utility.cpp
    mw/dcc/accessory.cpp
    mw/dcc/loco.cpp
    mw/dcc/service.cpp
    mw/dcc/system_state.cpp
    mw/dcc/turnout.cpp
    mw/zimo/ulf/dcc_ein/task_function.cpp
    utility.cpp)

if(IDF_TARGET STREQUAL linux)
  idf_component_register(
    SRCS
    ${COMMON_SRC}
    INCLUDE_DIRS
    .
    ${CMAKE_BINARY_DIR}
    REQUIRES
    bblanchon__arduinojson
    driver
    esp_app_format
    esp_http_server
    nvs_flash
    EMBED_FILES
    ${CMAKE_BINARY_DIR}/intf/http/ap/captive_portal.html)
else()
  idf_component_register(
    SRCS
    ${COMMON_SRC}
    drv/analog/adc_task_function.cpp
    drv/analog/convert.cpp
    drv/analog/init.cpp
    drv/analog/temp_task_function.cpp
    drv/led/bug.cpp
    drv/led/init.cpp
    drv/led/wifi.cpp
    drv/out/susi/init.cpp
    drv/out/susi/zimo/zusi/resume.cpp
    drv/out/susi/zimo/zusi/suspend.cpp
    drv/out/susi/zimo/zusi/task_function.cpp
    drv/out/track/dcc/resume.cpp
    drv/out/track/dcc/suspend.cpp
    drv/out/track/dcc/task_function.cpp
    drv/out/track/zimo/decup/resume.cpp
    drv/out/track/zimo/decup/suspend.cpp
    drv/out/track/zimo/decup/task_function.cpp
    drv/out/track/zimo/mdu/resume.cpp
    drv/out/track/zimo/mdu/suspend.cpp
    drv/out/track/zimo/mdu/task_function.cpp
    drv/out/track/init.cpp
    drv/out/init.cpp
    drv/out/suspend.cpp
    drv/trace/init.cpp
    drv/wifi/init.cpp
    drv/wifi/task_function.cpp
    intf/http/ap/init.cpp
    intf/http/ap/server.cpp
    intf/http/ap/dns.cpp
    intf/http/sta/init.cpp
    intf/http/sta/server.cpp
    intf/http/init.cpp
    intf/mdns/init.cpp
    intf/udp/init.cpp
    intf/usb/init.cpp
    intf/usb/rx_task_function.cpp
    intf/usb/tx_task_function.cpp
    mw/dcc/init.cpp
    mw/ota/init.cpp
    mw/ota/service.cpp
    mw/roco/z21/init.cpp
    mw/roco/z21/service.cpp
    mw/zimo/decup/init.cpp
    mw/zimo/decup/service.cpp
    mw/zimo/mdu/init.cpp
    mw/zimo/mdu/service.cpp
    mw/zimo/ulf/decup_ein/task_function.cpp
    mw/zimo/ulf/susiv2/task_function.cpp
    mw/zimo/ulf/init.cpp
    mw/zimo/zusi/init.cpp
    mw/zimo/zusi/service.cpp
    app_main.cpp
    INCLUDE_DIRS
    .
    ${CMAKE_BINARY_DIR}
    REQUIRES
    app_update
    bootloader_support
    driver
    esp_adc
    esp_app_format
    esp_http_server
    esp_wifi
    lwip
    nvs_flash
    vfs
    EMBED_FILES
    ${CMAKE_BINARY_DIR}/intf/http/ap/captive_portal.html)
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

cpmaddpackage("gh:ZIMO-Elektronik/ULF_COM@0.1.1")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_DCC_EIN@0.5.0")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_DECUP_EIN@0.0.1")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_MDU_EIN@0.0.0")
cpmaddpackage("gh:ZIMO-Elektronik/ULF_SUSIV2@0.2.0")
cpmaddpackage("gh:ZIMO-Elektronik/ZUSI@0.9.2")
cpmaddpackage("gh:microsoft/GSL@4.2.0")
cpmaddpackage(
  NAME
  Boost
  VERSION
  1.88.0
  URL
  https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.gz
  URL_HASH
  SHA256=dcea50f40ba1ecfc448fdf886c0165cf3e525fef2c9e3e080b9804e8117b9694
  OPTIONS
  "BOOST_ENABLE_CMAKE ON"
  "BOOST_SKIP_INSTALL_RULES ON"
  "BUILD_SHARED_LIBS OFF"
  "BOOST_INCLUDE_LIBRARIES preprocessor")

# According to this answer here https://stackoverflow.com/a/55312360/5840652
# magic enum works up to INT16_MAX
cpmaddpackage("gh:Neargye/magic_enum@0.9.7")
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
  11.2.0
  SYSTEM
  YES
  OPTIONS
  "FMT_INSTALL OFF"
  "FMT_OS OFF")

target_link_libraries(
  ${COMPONENT_LIB}
  PUBLIC Boost::preprocessor
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
minify_html(intf/http/ap/captive_portal.html
            ${CMAKE_BINARY_DIR}/intf/http/ap/captive_portal.html)

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
