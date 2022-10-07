// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/base_client.h>
#include <buzzblog/gen/TPostService.h>

#include <string>

using namespace gen;

namespace post_service {
class Client : public BaseClient<TPostServiceClient> {
 public:
  Client(const std::string& ip_address, const int port,
         const int conn_timeout_ms)
      : BaseClient<TPostServiceClient>(ip_address, port, conn_timeout_ms) {}

  TPost create_post(const TRequestMetadata& request_metadata,
                    const std::string& text) {
    TPost _return;
    _client->create_post(_return, request_metadata, text);
    return _return;
  }

  TPost retrieve_standard_post(const TRequestMetadata& request_metadata,
                               const int32_t post_id) {
    TPost _return;
    _client->retrieve_standard_post(_return, request_metadata, post_id);
    return _return;
  }

  TPost retrieve_expanded_post(const TRequestMetadata& request_metadata,
                               const int32_t post_id) {
    TPost _return;
    _client->retrieve_expanded_post(_return, request_metadata, post_id);
    return _return;
  }

  void delete_post(const TRequestMetadata& request_metadata,
                   const int32_t post_id) {
    _client->delete_post(request_metadata, post_id);
  }

  std::vector<TPost> list_posts(const TRequestMetadata& request_metadata,
                                const TPostQuery& query, const int32_t limit,
                                const int32_t offset) {
    std::vector<TPost> _return;
    _client->list_posts(_return, request_metadata, query, limit, offset);
    return _return;
  }

  int32_t count_posts_by_author(const TRequestMetadata& request_metadata,
                                const int32_t author_id) {
    return _client->count_posts_by_author(request_metadata, author_id);
  }
};
}  // namespace post_service
