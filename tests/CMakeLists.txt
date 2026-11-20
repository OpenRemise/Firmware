include(GoogleTest)

file(GLOB_RECURSE SRC *.c *.cpp)
idf_component_register(
  SRCS
  ${SRC}
  INCLUDE_DIRS
  .
  REQUIRES
  src
  WHOLE_ARCHIVE)

idf_build_set_property(COMPILE_OPTIONS -Wno-error=format APPEND)
idf_build_set_property(COMPILE_OPTIONS -Wno-error=undef APPEND)

cpmaddpackage(URI "gh:google/googletest#main" OPTIONS "INSTALL_GTEST OFF")

# GoogleTest doesn't correctly detect whether pthread is available
target_compile_definitions(gtest PUBLIC GTEST_HAS_PTHREAD=1)
target_compile_definitions(gmock PUBLIC GTEST_HAS_PTHREAD=1)

target_link_libraries(${COMPONENT_LIB} PRIVATE GTest::gtest GTest::gmock)

# Get esp-emulator
cpmaddpackage(
  NAME
  esp-emulator
  URL
  "https://github.com/espressif/esp-emulator/releases/download/v1.2.0/esp-emu-1.2.0-x86_64-unknown-linux-gnu.tar.gz"
  VERSION
  1.2.0
  DOWNLOAD_ONLY
  ON)
