// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <spdlog/sinks/basic_file_sink.h>

#include <buzzblog/gen/TPostService.h>
#include <buzzblog/base_client.h>


using namespace gen;


namespace post_service {
  class Client : public BaseClient<TPostServiceClient> {
   public:
    Client(const std::string& ip_address, const int port,
        const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger)
    : BaseClient<TPostServiceClient>(ip_address, port, conn_timeout_ms,
        logger) {
    }

    TPost create_post(const TRequestMetadata& request_metadata,
        const std::string& text) {
      TPost _return;
      VOID_RPC_WRAPPER(
          std::bind(&TPostServiceClient::create_post, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(text)),
          request_metadata.id, "post:create_post");
      return _return;
    }

    TPost retrieve_standard_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      TPost _return;
      VOID_RPC_WRAPPER(
          std::bind(&TPostServiceClient::retrieve_standard_post, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(post_id)),
          request_metadata.id, "post:retrieve_standard_post");
      return _return;
    }

    TPost retrieve_expanded_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      TPost _return;
      VOID_RPC_WRAPPER(
          std::bind(&TPostServiceClient::retrieve_expanded_post, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(post_id)),
          request_metadata.id, "post:retrieve_expanded_post");
      return _return;
    }

    void delete_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      VOID_RPC_WRAPPER(
          std::bind(&TPostServiceClient::delete_post, _client,
              std::ref(request_metadata), std::ref(post_id)),
          request_metadata.id, "post:delete_post");
    }

    std::vector<TPost> list_posts(const TRequestMetadata& request_metadata,
        const TPostQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TPost> _return;
      VOID_RPC_WRAPPER(
          std::bind(&TPostServiceClient::list_posts, _client, std::ref(_return),
              std::ref(request_metadata), std::ref(query), std::ref(limit),
              std::ref(offset)),
          request_metadata.id, "post:list_posts");
      return _return;
    }

    int32_t count_posts_by_author(const TRequestMetadata& request_metadata,
        const int32_t author_id) {
      return RPC_WRAPPER<int32_t>(
          std::bind(&TPostServiceClient::count_posts_by_author, _client,
              std::ref(request_metadata), std::ref(author_id)),
          request_metadata.id, "post:count_posts_by_author");
    }
  };
}
