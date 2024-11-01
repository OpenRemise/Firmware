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

cpmaddpackage(
  NAME
  googletest
  GITHUB_REPOSITORY
  "google/googletest"
  GIT_TAG
  main
  SYSTEM
  YES
  OPTIONS
  "INSTALL_GTEST OFF")

target_link_libraries(${COMPONENT_LIB} PRIVATE GTest::gtest GTest::gmock)
