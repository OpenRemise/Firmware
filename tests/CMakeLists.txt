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

cpmaddpackage(URI "gh:google/googletest#main" OPTIONS "INSTALL_GTEST OFF")

target_link_libraries(${COMPONENT_LIB} PRIVATE GTest::gtest GTest::gmock)
