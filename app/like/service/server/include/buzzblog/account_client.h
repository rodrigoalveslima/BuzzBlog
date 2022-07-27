// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <spdlog/sinks/basic_file_sink.h>

#include <buzzblog/gen/TAccountService.h>
#include <buzzblog/base_client.h>


using namespace gen;


namespace account_service {
  class Client : public BaseClient<TAccountServiceClient> {
   public:
    Client(const std::string& ip_address, const int port,
        const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger)
    : BaseClient<TAccountServiceClient>(ip_address, port, conn_timeout_ms,
        logger) {
    }

    TAccount authenticate_user(const TRequestMetadata& request_metadata,
        const std::string& username, const std::string& password) {
      TAccount _return;
      VOID_RPC_WRAPPER(
          std::bind(&TAccountServiceClient::authenticate_user, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(username),
              std::ref(password)),
          request_metadata.id, "account:authenticate_user");
      return _return;
    }

    TAccount create_account(const TRequestMetadata& request_metadata,
        const std::string& username, const std::string& password,
        const std::string& first_name, const std::string& last_name) {
      TAccount _return;
      VOID_RPC_WRAPPER(
          std::bind(&TAccountServiceClient::create_account, _client,
              std::ref(_return), std::ref(request_metadata), std::ref(username),
              std::ref(password), std::ref(first_name), std::ref(last_name)),
          request_metadata.id, "account:create_account");
      return _return;
    }

    TAccount retrieve_standard_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      TAccount _return;
      VOID_RPC_WRAPPER(
          std::bind(&TAccountServiceClient::retrieve_standard_account, _client,
              std::ref(_return), std::ref(request_metadata),
              std::ref(account_id)),
          request_metadata.id, "account:retrieve_standard_account");
      return _return;
    }

    TAccount retrieve_expanded_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      TAccount _return;
      VOID_RPC_WRAPPER(
          std::bind(&TAccountServiceClient::retrieve_expanded_account, _client,
              std::ref(_return), std::ref(request_metadata),
              std::ref(account_id)),
          request_metadata.id, "account:retrieve_expanded_account");
      return _return;
    }

    TAccount update_account(const TRequestMetadata& request_metadata,
        const int32_t account_id, const std::string& password,
        const std::string& first_name, const std::string& last_name) {
      TAccount _return;
      VOID_RPC_WRAPPER(
          std::bind(&TAccountServiceClient::update_account, _client,
              std::ref(_return), std::ref(request_metadata),
              std::ref(account_id), std::ref(password), std::ref(first_name),
              std::ref(last_name)),
          request_metadata.id, "account:update_account");
      return _return;
    }

    void delete_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      VOID_RPC_WRAPPER(
          std::bind(&TAccountServiceClient::delete_account, _client,
              std::ref(request_metadata), std::ref(account_id)),
          request_metadata.id, "account:delete_account");
    }

    std::vector<TAccount> list_accounts(const TRequestMetadata& request_metadata,
        const TAccountQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TAccount> _return;
      VOID_RPC_WRAPPER(
          std::bind(&TAccountServiceClient::list_accounts, _client, std::ref(_return),
              std::ref(request_metadata), std::ref(query), std::ref(limit),
              std::ref(offset)),
          request_metadata.id, "account:list_accounts");
      return _return;
    }
  };
}
