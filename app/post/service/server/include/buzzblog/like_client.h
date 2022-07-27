// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <spdlog/sinks/basic_file_sink.h>

#include <buzzblog/gen/TLikeService.h>
#include <buzzblog/base_client.h>


using namespace gen;


namespace like_service {
  class Client : public BaseClient<TLikeServiceClient> {
   public:
    Client(const std::string& ip_address, const int port,
        const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger)
    : BaseClient<TLikeServiceClient>(ip_address, port, conn_timeout_ms,
        logger) {
    }

    TLike like_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      TLike _return;
      VOID_RPC_WRAPPER(
          std::bind(&TLikeServiceClient::like_post, _client, std::ref(_return),
              std::ref(request_metadata), std::ref(post_id)),
          request_metadata.id, "like:like_post");
      return _return;
    }

    TLike retrieve_standard_like(const TRequestMetadata& request_metadata,
        const int32_t like_id) {
      TLike _return;
      VOID_RPC_WRAPPER(
          std::bind(&TLikeServiceClient::retrieve_standard_like, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(like_id)),
          request_metadata.id, "like:retrieve_standard_like");
      return _return;
    }

    TLike retrieve_expanded_like(const TRequestMetadata& request_metadata,
        const int32_t like_id) {
      TLike _return;
      VOID_RPC_WRAPPER(
          std::bind(&TLikeServiceClient::retrieve_expanded_like, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(like_id)),
          request_metadata.id, "like:retrieve_expanded_like");
      return _return;
    }

    void delete_like(const TRequestMetadata& request_metadata,
        const int32_t like_id) {
      VOID_RPC_WRAPPER(
          std::bind(&TLikeServiceClient::delete_like, _client,
              std::ref(request_metadata), std::ref(like_id)),
          request_metadata.id, "like:delete_like");
    }

    std::vector<TLike> list_likes(const TRequestMetadata& request_metadata,
        const TLikeQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TLike> _return;
      VOID_RPC_WRAPPER(
          std::bind(&TLikeServiceClient::list_likes, _client, std::ref(_return),
              std::ref(request_metadata), std::ref(query), std::ref(limit),
              std::ref(offset)),
          request_metadata.id, "like:list_likes");
      return _return;
    }

    int32_t count_likes_by_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      return RPC_WRAPPER<int32_t>(
          std::bind(&TLikeServiceClient::count_likes_by_account, _client,
              std::ref(request_metadata), std::ref(account_id)),
          request_metadata.id, "like:count_likes_by_account");
    }

    int32_t count_likes_of_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      return RPC_WRAPPER<int32_t>(
          std::bind(&TLikeServiceClient::count_likes_of_post, _client,
              std::ref(request_metadata), std::ref(post_id)),
          request_metadata.id, "like:count_likes_of_post");
    }
  };
}
