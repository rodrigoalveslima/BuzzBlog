// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/base_client.h>
#include <buzzblog/gen/TAccountService.h>

#include <string>

using namespace gen;

namespace account_service {
class Client : public BaseClient<TAccountServiceClient> {
 public:
  Client(const std::string& ip_address, const int port,
         const int conn_timeout_ms)
      : BaseClient<TAccountServiceClient>(ip_address, port, conn_timeout_ms) {}

  TAccount authenticate_user(const TRequestMetadata& request_metadata,
                             const std::string& username,
                             const std::string& password) {
    TAccount _return;
    _client->authenticate_user(_return, request_metadata, username, password);
    return _return;
  }

  TAccount create_account(const TRequestMetadata& request_metadata,
                          const std::string& username,
                          const std::string& password,
                          const std::string& first_name,
                          const std::string& last_name) {
    TAccount _return;
    _client->create_account(_return, request_metadata, username, password,
                            first_name, last_name);
    return _return;
  }

  TAccount retrieve_standard_account(const TRequestMetadata& request_metadata,
                                     const int32_t account_id) {
    TAccount _return;
    _client->retrieve_standard_account(_return, request_metadata, account_id);
    return _return;
  }

  TAccount retrieve_expanded_account(const TRequestMetadata& request_metadata,
                                     const int32_t account_id) {
    TAccount _return;
    _client->retrieve_expanded_account(_return, request_metadata, account_id);
    return _return;
  }

  TAccount update_account(const TRequestMetadata& request_metadata,
                          const int32_t account_id, const std::string& password,
                          const std::string& first_name,
                          const std::string& last_name) {
    TAccount _return;
    _client->update_account(_return, request_metadata, account_id, password,
                            first_name, last_name);
    return _return;
  }

  void delete_account(const TRequestMetadata& request_metadata,
                      const int32_t account_id) {
    _client->delete_account(request_metadata, account_id);
  }

  std::vector<TAccount> list_accounts(const TRequestMetadata& request_metadata,
                                      const TAccountQuery& query,
                                      const int32_t limit,
                                      const int32_t offset) {
    std::vector<TAccount> _return;
    _client->list_accounts(_return, request_metadata, query, limit, offset);
    return _return;
  }
};
}  // namespace account_service
