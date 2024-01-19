#pragma once

#include <gtest/gtest.h>

// USB test fixture
class UsbTest : public virtual ::testing::Test {
public:
  UsbTest();
  ~UsbTest();

  std::array<char, usb::buffer_size> _stack{};
};
