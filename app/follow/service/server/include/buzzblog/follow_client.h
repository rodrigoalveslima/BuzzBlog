// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <spdlog/sinks/basic_file_sink.h>

#include <buzzblog/gen/TFollowService.h>
#include <buzzblog/base_client.h>


using namespace gen;


namespace follow_service {
  class Client : public BaseClient<TFollowServiceClient> {
   public:
    Client(const std::string& ip_address, const int port,
        const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger)
    : BaseClient<TFollowServiceClient>(ip_address, port, conn_timeout_ms,
        logger) {
    }

    TFollow follow_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      TFollow _return;
      VOID_RPC_WRAPPER(
          std::bind(&TFollowServiceClient::follow_account, _client,
              std::ref(_return), std::ref(request_metadata),
              std::ref(account_id)),
          request_metadata.id, "follow:follow_account");
      return _return;
    }

    TFollow retrieve_standard_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      TFollow _return;
      VOID_RPC_WRAPPER(
          std::bind(&TFollowServiceClient::retrieve_standard_follow, _client,
              std::ref(_return), std::ref(request_metadata),
              std::ref(follow_id)),
          request_metadata.id, "follow:retrieve_standard_follow");
      return _return;
    }

    TFollow retrieve_expanded_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      TFollow _return;
      VOID_RPC_WRAPPER(
          std::bind(&TFollowServiceClient::retrieve_expanded_follow, _client,
              std::ref(_return), std::ref(request_metadata),
              std::ref(follow_id)),
          request_metadata.id, "follow:retrieve_expanded_follow");
      return _return;
    }

    void delete_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      VOID_RPC_WRAPPER(
          std::bind(&TFollowServiceClient::delete_follow, _client,
              std::ref(request_metadata), std::ref(follow_id)),
          request_metadata.id, "follow:delete_follow");
    }

    std::vector<TFollow> list_follows(const TRequestMetadata& request_metadata,
        const TFollowQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TFollow> _return;
      VOID_RPC_WRAPPER(
          std::bind(&TFollowServiceClient::list_follows, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(query),
              std::ref(limit), std::ref(offset)),
          request_metadata.id, "follow:list_follows");
      return _return;
    }

    bool check_follow(const TRequestMetadata& request_metadata,
        const int32_t follower_id, const int32_t followee_id) {
      return RPC_WRAPPER<bool>(
          std::bind(&TFollowServiceClient::check_follow, _client,
              std::ref(request_metadata), std::ref(follower_id),
              std::ref(followee_id)),
          request_metadata.id, "follow:check_follow");
    }

    int32_t count_followers(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      return RPC_WRAPPER<int32_t>(
          std::bind(&TFollowServiceClient::count_followers, _client,
              std::ref(request_metadata), std::ref(account_id)),
          request_metadata.id, "follow:count_followers");
    }

    int32_t count_followees(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      return RPC_WRAPPER<int32_t>(
          std::bind(&TFollowServiceClient::count_followees, _client,
              std::ref(request_metadata), std::ref(account_id)),
          request_metadata.id, "follow:count_followees");
    }
  };
}
