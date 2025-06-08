#include "endpoints_map_test.hpp"

TEST_F(EndpointsKeyTest, equal_although_uri_different) {
  key_type a{"/dcc/", DELETE};
  key_type b{"/dcc/3", DELETE};

  _map[a] = 1;
  _map[b] = 2;
  EXPECT_EQ(size(_map), 1uz);

  EXPECT_TRUE(_map.contains(a));
  EXPECT_TRUE(_map.contains(b));
}

TEST_F(EndpointsKeyTest, not_equal_different_uri) {
  key_type a{"/dcc/", DELETE};
  key_type b{"/about/", DELETE};

  _map[a] = 1;
  _map[b] = 2;
  EXPECT_EQ(size(_map), 2uz);

  EXPECT_TRUE(_map.contains(a));
  EXPECT_TRUE(_map.contains(b));
}

TEST_F(EndpointsKeyTest, not_equal_different_method) {
  key_type a{"/dcc/", DELETE};
  key_type b{"/dcc/", GET};

  _map[a] = 1;
  _map[b] = 2;
  EXPECT_EQ(size(_map), 2uz);

  EXPECT_TRUE(_map.contains(a));
  EXPECT_TRUE(_map.contains(b));
}

TEST_F(EndpointsKeyTest, not_equal_different_uri_and_method) {
  key_type a{"/dcc/", POST};
  key_type b{"/about/", GET};

  key_compare comp;
  EXPECT_FALSE(comp(a, a));
  EXPECT_FALSE(comp(b, b));
  EXPECT_NE(comp(a, b), comp(b, a));

  _map[a] = 1;
  _map[b] = 1;
  EXPECT_EQ(size(_map), 2uz);

  EXPECT_TRUE(_map.contains(a));
  EXPECT_TRUE(_map.contains(b));
}
