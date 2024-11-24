#pragma once

#include <gtest/gtest.h>
#include "dcc/service.hpp"

// DCC test fixture
class DccTest : public virtual ::testing::Test {
public:
  DccTest();
  ~DccTest();

protected:
  // std::shared_ptr<dcc::Service> _service{
  //   std::make_shared<dcc::Service>(tskNO_AFFINITY)};
};
