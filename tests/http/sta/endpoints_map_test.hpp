#pragma once

#include <gtest/gtest.h>
#include <map>

//
class EndpointsKeyTest : public virtual ::testing::Test {
protected:
  enum httpd_method_t { DELETE, GET, HEAD, POST, PUT };

  struct key_type {
    char const* uri;
    httpd_method_t method;
  };

  struct key_compare {
    bool operator()(key_type const& lhs, key_type const& rhs) const {
      return lhs.method != rhs.method
               ? lhs.method < rhs.method
               : strncmp(lhs.uri,
                         rhs.uri,
                         std::min(strlen(lhs.uri), strlen(rhs.uri))) < 0;
    }
  };

  std::map<key_type, int, key_compare> _map;
};
