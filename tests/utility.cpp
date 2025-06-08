#include "utility.hpp"
#include <gtest/gtest.h>

TEST(utility, decode_uri) {
  std::string_view url{"http%3A%2F%2Ffoo%20bar%2F"};
  std::string decoded(size(url), '\0');
  auto last{decode_uri(url, begin(decoded))};
  decoded.resize(last - cbegin(decoded));
  EXPECT_EQ(decoded, "http://foo bar/");
}

TEST(utility, make_unique_caps) {
  [[maybe_unused]] auto heap{make_unique_caps<char>(42uz, MALLOC_CAP_SPIRAM)};
}

TEST(utility, validate_json) {
  std::string_view valid{"{\"address\":100,\"name\":\"MoFo\",\"functions\":"
                         "1234,\"speed\":42,\"dir\":1}"};
  EXPECT_TRUE(validate_json(valid));

  std::string_view invalid{
    "{\"address\":100:\"MoFo\",\"functions\":1234,\"speed\":42,\"dir\":1}"};
  EXPECT_FALSE(validate_json(invalid));
}

TEST(utility, uri2address) {
  EXPECT_EQ(uri2address("http://remise.local/dcc/locos/3"), 3u);
  EXPECT_EQ(uri2address("http://remise.local/dcc/locos/42"), 42u);
  EXPECT_FALSE(uri2address("http://remise.local/dcc/locos/x4-2"));
}

TEST(utility, uri2loco_address) {
  auto addr_basic_loco{uri2loco_address("http://remise.local/dcc/locos/3")};
  EXPECT_EQ(addr_basic_loco, 3);
  EXPECT_EQ(addr_basic_loco->type, dcc::Address::BasicLoco);
  auto addr_ext_loco{uri2loco_address("http://remise.local/dcc/locos/266")};
  EXPECT_EQ(addr_ext_loco, 266);
  EXPECT_EQ(addr_ext_loco->type, dcc::Address::ExtendedLoco);
  EXPECT_FALSE(uri2address("http://remise.local/dcc/locos/.dk182"));
}
