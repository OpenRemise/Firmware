#include "endpoints_test.hpp"

using namespace testing;

TEST_F(EndpointsTest, uri_without_slash_triggers_assertion) {
  ASSERT_DEATH(subscribe({.uri = "no_slash", .method = HTTP_GET},
                         _subscriber,
                         &SubscriberMock::request0),
               "");
}

TEST_F(EndpointsTest, same_method_different_uri) {
  EXPECT_CALL(*_subscriber, request0(_)).Times(Exactly(1));
  EXPECT_CALL(*_subscriber, request1(_)).Times(Exactly(1));

  subscribe(
    {.uri = "/a/", .method = HTTP_GET}, _subscriber, &SubscriberMock::request0);
  subscribe(
    {.uri = "/b/", .method = HTTP_GET}, _subscriber, &SubscriberMock::request1);

  {
    httpd_req_t req{.handle = NULL, .method = HTTP_GET, .uri = "/a/"};
    syncResponse(&req);
  }

  {
    httpd_req_t req{.handle = NULL, .method = HTTP_GET, .uri = "/b/"};
    syncResponse(&req);
  }
}

TEST_F(EndpointsTest, same_method_same_uri_prefix) {
  EXPECT_CALL(*_subscriber, request0(_)).Times(Exactly(1));
  EXPECT_CALL(*_subscriber, request1(_)).Times(Exactly(2));

  subscribe({.uri = "/dcc/", .method = HTTP_GET},
            _subscriber,
            &SubscriberMock::request0);
  subscribe({.uri = "/dcc/locos/", .method = HTTP_GET},
            _subscriber,
            &SubscriberMock::request1);

  {
    httpd_req_t req{.handle = NULL, .method = HTTP_GET, .uri = "/dcc/"};
    syncResponse(&req);
  }

  {
    httpd_req_t req{.handle = NULL, .method = HTTP_GET, .uri = "/dcc/locos/"};
    syncResponse(&req);
  }

  {
    httpd_req_t req{.handle = NULL, .method = HTTP_GET, .uri = "/dcc/locos/3"};
    syncResponse(&req);
  }
}

TEST_F(EndpointsTest, different_method_same_uri) {
  EXPECT_CALL(*_subscriber, request0(_)).Times(Exactly(1));
  EXPECT_CALL(*_subscriber, request1(_)).Times(Exactly(1));

  subscribe(
    {.uri = "/a/", .method = HTTP_GET}, _subscriber, &SubscriberMock::request0);
  subscribe({.uri = "/a/", .method = HTTP_POST},
            _subscriber,
            &SubscriberMock::request1);

  {
    httpd_req_t req{.handle = NULL, .method = HTTP_GET, .uri = "/a/"};
    syncResponse(&req);
  }

  {
    httpd_req_t req{.handle = NULL, .method = HTTP_POST, .uri = "/a/"};
    syncResponse(&req);
  }
}
