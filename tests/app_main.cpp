#include <gtest/gtest.h>
#include <pthread.h>

namespace {

void* run_tests(void*) {
  int argc{1};
  char arg0[]{"FirmwareTests"};
  char* argv[]{arg0, nullptr};
  testing::InitGoogleTest(&argc, argv);
  return reinterpret_cast<void*>(static_cast<intptr_t>(RUN_ALL_TESTS()));
}

} // namespace

extern "C" void app_main() {
  // GoogleTest needs considerably more stack than a typical small task.
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 64 * 1024);

  // GoogleTest needs to run in a a POSIX pthread
  pthread_t thread;
  pthread_create(&thread, &attr, run_tests, nullptr);
  void* result;
  pthread_join(thread, &result);
  printf("TEST_RESULT:%d\n", reinterpret_cast<intptr_t>(result));
}
