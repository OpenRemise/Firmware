#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "intf/http/endpoints.hpp"

struct SubscriberMock {
  MOCK_METHOD(intf::http::Response, request0, (intf::http::Request const&));
  MOCK_METHOD(intf::http::Response, request1, (intf::http::Request const&));
};
