// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <spdlog/sinks/basic_file_sink.h>

#include <buzzblog/gen/TWordfilterService.h>
#include <buzzblog/base_client.h>


using namespace gen;


namespace wordfilter_service {
  class Client : public BaseClient<TWordfilterServiceClient> {
   public:
    Client(const std::string& ip_address, const int port,
        const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger)
    : BaseClient<TWordfilterServiceClient>(ip_address, port, conn_timeout_ms,
        logger) {
    }

    bool is_valid_word(const TRequestMetadata& request_metadata,
        const std::string& word) {
      return RPC_WRAPPER<bool>(
          std::bind(&TWordfilterServiceClient::is_valid_word, _client,
              std::ref(request_metadata), std::ref(word)),
          request_metadata.id, "wordfilter:is_valid_word");
    }
  };
}