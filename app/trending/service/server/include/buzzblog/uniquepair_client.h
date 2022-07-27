// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <spdlog/sinks/basic_file_sink.h>

#include <buzzblog/gen/TUniquepairService.h>
#include <buzzblog/base_client.h>


using namespace gen;


namespace uniquepair_service {
  class Client : public BaseClient<TUniquepairServiceClient> {
   public:
    Client(const std::string& ip_address, const int port,
        const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger)
    : BaseClient<TUniquepairServiceClient>(ip_address, port, conn_timeout_ms,
        logger) {
    }

    TUniquepair get(const TRequestMetadata& request_metadata,
        const int32_t uniquepair_id) {
      TUniquepair _return;
      VOID_RPC_WRAPPER(
          std::bind(&TUniquepairServiceClient::get, _client, std::ref(_return),
              std::ref(request_metadata), std::ref(uniquepair_id)),
          request_metadata.id, "uniquepair:get");
      return _return;
    }

    TUniquepair add(const TRequestMetadata& request_metadata,
        const std::string& domain, const int32_t first_elem,
        const int32_t second_elem) {
      TUniquepair _return;
      VOID_RPC_WRAPPER(
          std::bind(&TUniquepairServiceClient::add, _client, std::ref(_return),
              std::ref(request_metadata), std::ref(domain),
              std::ref(first_elem), std::ref(second_elem)),
          request_metadata.id, "uniquepair:add");
      return _return;
    }

    void remove(const TRequestMetadata& request_metadata,
        const int32_t uniquepair_id) {
      VOID_RPC_WRAPPER(
          std::bind(&TUniquepairServiceClient::remove, _client,
              std::ref(request_metadata), std::ref(uniquepair_id)),
          request_metadata.id, "uniquepair:remove");
    }

    TUniquepair find(const TRequestMetadata& request_metadata,
        const std::string& domain, const int32_t first_elem,
        const int32_t second_elem) {
      TUniquepair _return;
      VOID_RPC_WRAPPER(
          std::bind(&TUniquepairServiceClient::find, _client, std::ref(_return),
              std::ref(request_metadata), std::ref(domain),
              std::ref(first_elem), std::ref(second_elem)),
          request_metadata.id, "uniquepair:find");
      return _return;
    }

    std::vector<TUniquepair> fetch(const TRequestMetadata& request_metadata,
        const TUniquepairQuery& query, const int32_t limit,
        const int32_t offset) {
      std::vector<TUniquepair> _return;
      VOID_RPC_WRAPPER(
          std::bind(&TUniquepairServiceClient::fetch, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(query),
              std::ref(limit), std::ref(offset)),
          request_metadata.id, "uniquepair:fetch");
      return _return;
    }

    int32_t count(const TRequestMetadata& request_metadata,
        const TUniquepairQuery& query) {
      return RPC_WRAPPER<int32_t>(
          std::bind(&TUniquepairServiceClient::count, _client,
              std::ref(request_metadata), std::ref(query)),
          request_metadata.id, "uniquepair:count");
    }
  };
}
