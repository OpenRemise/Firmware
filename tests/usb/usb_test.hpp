#pragma once

#include <gtest/gtest.h>

// USB test fixture
class UsbTest : public virtual ::testing::Test {
public:
  UsbTest();
  ~UsbTest();
};
