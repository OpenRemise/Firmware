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

set(INSTALL_GTEST
    OFF
    CACHE BOOL "" FORCE)
cpmaddpackage("gh:google/googletest#main")

target_link_libraries(${COMPONENT_LIB} PRIVATE GTest::gtest GTest::gmock
                                               MDU::MDU)
