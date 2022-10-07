// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/base_client.h>
#include <buzzblog/gen/TLikeService.h>

#include <string>

using namespace gen;

namespace like_service {
class Client : public BaseClient<TLikeServiceClient> {
 public:
  Client(const std::string& ip_address, const int port,
         const int conn_timeout_ms)
      : BaseClient<TLikeServiceClient>(ip_address, port, conn_timeout_ms) {}

  TLike like_post(const TRequestMetadata& request_metadata,
                  const int32_t post_id) {
    TLike _return;
    _client->like_post(_return, request_metadata, post_id);
    return _return;
  }

  TLike retrieve_standard_like(const TRequestMetadata& request_metadata,
                               const int32_t like_id) {
    TLike _return;
    _client->retrieve_standard_like(_return, request_metadata, like_id);
    return _return;
  }

  TLike retrieve_expanded_like(const TRequestMetadata& request_metadata,
                               const int32_t like_id) {
    TLike _return;
    _client->retrieve_expanded_like(_return, request_metadata, like_id);
    return _return;
  }

  void delete_like(const TRequestMetadata& request_metadata,
                   const int32_t like_id) {
    _client->delete_like(request_metadata, like_id);
  }

  std::vector<TLike> list_likes(const TRequestMetadata& request_metadata,
                                const TLikeQuery& query, const int32_t limit,
                                const int32_t offset) {
    std::vector<TLike> _return;
    _client->list_likes(_return, request_metadata, query, limit, offset);
    return _return;
  }

  int32_t count_likes_by_account(const TRequestMetadata& request_metadata,
                                 const int32_t account_id) {
    return _client->count_likes_by_account(request_metadata, account_id);
  }

  int32_t count_likes_of_post(const TRequestMetadata& request_metadata,
                              const int32_t post_id) {
    return _client->count_likes_of_post(request_metadata, post_id);
  }
};
}  // namespace like_service
