// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/base_client.h>
#include <buzzblog/gen/TWordfilterService.h>

#include <string>

using namespace gen;

namespace wordfilter_service {
class Client : public BaseClient<TWordfilterServiceClient> {
 public:
  Client(const std::string& ip_address, const int port,
         const int conn_timeout_ms)
      : BaseClient<TWordfilterServiceClient>(ip_address, port,
                                             conn_timeout_ms) {}

  bool is_valid_word(const TRequestMetadata& request_metadata,
                     const std::string& word) {
    return _client->is_valid_word(request_metadata, word);
  }
};
}  // namespace wordfilter_service
