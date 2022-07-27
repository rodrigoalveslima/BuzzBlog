// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>

#include <buzzblog/gen/TTrendingService.h>
#include <buzzblog/base_client.h>


using namespace gen;


namespace trending_service {
  class Client : public BaseClient<TTrendingServiceClient> {
   public:
    Client(const std::string& ip_address, const int port,
        const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger)
    : BaseClient<TTrendingServiceClient>(ip_address, port, conn_timeout_ms,
        logger) {
    }

    void process_post(const TRequestMetadata& request_metadata,
        const std::string& text) {
      VOID_RPC_WRAPPER(
          std::bind(&TTrendingServiceClient::process_post, _client,
              std::ref(request_metadata), std::ref(text)),
          request_metadata.id, "trending:process_post");
    }

    std::vector<std::string> fetch_trending_hashtags(const TRequestMetadata& request_metadata,
        const int32_t limit) {
      std::vector<std::string> _return;
      VOID_RPC_WRAPPER(
          std::bind(&TTrendingServiceClient::fetch_trending_hashtags, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(limit)),
          request_metadata.id, "trending:fetch_trending_hashtags");
      return _return;
    }
  };
}
