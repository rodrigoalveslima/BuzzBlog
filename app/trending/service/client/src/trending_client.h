// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/base_client.h>
#include <buzzblog/gen/TTrendingService.h>

#include <string>
#include <vector>

using namespace gen;

namespace trending_service {
class Client : public BaseClient<TTrendingServiceClient> {
 public:
  Client(const std::string& ip_address, const int port,
         const int conn_timeout_ms)
      : BaseClient<TTrendingServiceClient>(ip_address, port, conn_timeout_ms) {}

  void process_post(const TRequestMetadata& request_metadata,
                    const std::string& text) {
    _client->process_post(request_metadata, text);
  }

  std::vector<std::string> fetch_trending_hashtags(
      const TRequestMetadata& request_metadata, const int32_t limit) {
    std::vector<std::string> _return;
    _client->fetch_trending_hashtags(_return, request_metadata, limit);
    return _return;
  }
};
}  // namespace trending_service
