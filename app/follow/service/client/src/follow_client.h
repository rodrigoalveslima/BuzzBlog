// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/base_client.h>
#include <buzzblog/gen/TFollowService.h>

#include <string>

using namespace gen;

namespace follow_service {
class Client : public BaseClient<TFollowServiceClient> {
 public:
  Client(const std::string& ip_address, const int port,
         const int conn_timeout_ms)
      : BaseClient<TFollowServiceClient>(ip_address, port, conn_timeout_ms) {}

  TFollow follow_account(const TRequestMetadata& request_metadata,
                         const int32_t account_id) {
    TFollow _return;
    _client->follow_account(_return, request_metadata, account_id);
    return _return;
  }

  TFollow retrieve_standard_follow(const TRequestMetadata& request_metadata,
                                   const int32_t follow_id) {
    TFollow _return;
    _client->retrieve_standard_follow(_return, request_metadata, follow_id);
    return _return;
  }

  TFollow retrieve_expanded_follow(const TRequestMetadata& request_metadata,
                                   const int32_t follow_id) {
    TFollow _return;
    _client->retrieve_expanded_follow(_return, request_metadata, follow_id);
    return _return;
  }

  void delete_follow(const TRequestMetadata& request_metadata,
                     const int32_t follow_id) {
    _client->delete_follow(request_metadata, follow_id);
  }

  std::vector<TFollow> list_follows(const TRequestMetadata& request_metadata,
                                    const TFollowQuery& query,
                                    const int32_t limit, const int32_t offset) {
    std::vector<TFollow> _return;
    _client->list_follows(_return, request_metadata, query, limit, offset);
    return _return;
  }

  bool check_follow(const TRequestMetadata& request_metadata,
                    const int32_t follower_id, const int32_t followee_id) {
    return _client->check_follow(request_metadata, follower_id, followee_id);
  }

  int32_t count_followers(const TRequestMetadata& request_metadata,
                          const int32_t account_id) {
    return _client->count_followers(request_metadata, account_id);
  }

  int32_t count_followees(const TRequestMetadata& request_metadata,
                          const int32_t account_id) {
    return _client->count_followees(request_metadata, account_id);
  }
};
}  // namespace follow_service
