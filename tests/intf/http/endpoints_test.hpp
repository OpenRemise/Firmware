#pragma once

#include <gtest/gtest.h>
#include "intf/http/endpoints.hpp"
#include "subscriber_mock.hpp"

//
class EndpointsTest : public virtual ::testing::Test,
                      public intf::http::Endpoints {
protected:
  EndpointsTest();
  virtual ~EndpointsTest();

  std::shared_ptr<SubscriberMock> _subscriber{
    std::make_shared<SubscriberMock>()};
};
