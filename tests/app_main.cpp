#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

int argc;
char** argv;

// This is a dirty workaround to copy argc/argv before it disappears into the
// depths of ESP-IDF.
[[gnu::constructor]] void copy_argc_argv(int argc, char** argv) {
  ::argc = argc;
  ::argv = argv;
}

} // namespace

extern "C" void app_main() {
  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  testing::FLAGS_gtest_death_test_style = "threadsafe";
  exit(RUN_ALL_TESTS());
}