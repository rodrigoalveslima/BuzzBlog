// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/base_client.h>
#include <buzzblog/gen/TUniquepairService.h>

#include <string>

using namespace gen;

namespace uniquepair_service {
class Client : public BaseClient<TUniquepairServiceClient> {
 public:
  Client(const std::string& ip_address, const int port,
         const int conn_timeout_ms)
      : BaseClient<TUniquepairServiceClient>(ip_address, port,
                                             conn_timeout_ms) {}

  TUniquepair get(const TRequestMetadata& request_metadata,
                  const int32_t uniquepair_id) {
    TUniquepair _return;
    _client->get(_return, request_metadata, uniquepair_id);
    return _return;
  }

  TUniquepair add(const TRequestMetadata& request_metadata,
                  const std::string& domain, const int32_t first_elem,
                  const int32_t second_elem) {
    TUniquepair _return;
    _client->add(_return, request_metadata, domain, first_elem, second_elem);
    return _return;
  }

  void remove(const TRequestMetadata& request_metadata,
              const int32_t uniquepair_id) {
    _client->remove(request_metadata, uniquepair_id);
  }

  bool find(const TRequestMetadata& request_metadata, const std::string& domain,
            const int32_t first_elem, const int32_t second_elem) {
    return _client->find(request_metadata, domain, first_elem, second_elem);
  }

  std::vector<TUniquepair> fetch(const TRequestMetadata& request_metadata,
                                 const TUniquepairQuery& query,
                                 const int32_t limit, const int32_t offset) {
    std::vector<TUniquepair> _return;
    _client->fetch(_return, request_metadata, query, limit, offset);
    return _return;
  }

  int32_t count(const TRequestMetadata& request_metadata,
                const TUniquepairQuery& query) {
    return _client->count(request_metadata, query);
  }
};
}  // namespace uniquepair_service
