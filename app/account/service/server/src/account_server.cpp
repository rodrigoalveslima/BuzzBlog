// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <future>
#include <string>

#include <cxxopts.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TAccountService.h>
#include <buzzblog/microservice_connected_server.h>
#include <buzzblog/postgres_connected_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;


class TAccountServiceHandler :
    public MicroserviceConnectedServer,
    public PostgresConnectedServer,
    public TAccountServiceIf {
private:
  bool validate_attributes(const std::string& username,
      const std::string& password, const std::string& first_name,
      const std::string& last_name) {
    return (username.size() > 0 && username.size() <= 32 &&
        password.size() > 0 && password.size() <= 32 &&
        first_name.size() > 0 && first_name.size() <= 32 &&
        last_name.size() > 0 && last_name.size() <= 32);
  }

  std::string build_where_clause(const TAccountQuery& query) {
    std::ostringstream where_clause;
    where_clause << "active = true";
    if (query.__isset.username)
      where_clause << " AND username = '" << query.username << "'";
    return where_clause.str();
  }

public:
  TAccountServiceHandler(const std::string& backend_filepath,
      const int microservice_connection_pool_size,
      const int postgres_connection_pool_size,
      const std::string& postgres_user, const std::string& postgres_password)
  : MicroserviceConnectedServer(backend_filepath, microservice_connection_pool_size),
    PostgresConnectedServer(backend_filepath, postgres_connection_pool_size, postgres_user, postgres_password) {
  }

  void authenticate_user(TAccount& _return,
      const TRequestMetadata& request_metadata, const std::string& username,
      const std::string& password) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at, active, password, first_name, last_name "
        "FROM Accounts "
        "WHERE username = '%s'";
    sprintf(query_str, query_fmt, username.c_str());

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "account");

    // Check if account exists.
    if (db_res.begin() == db_res.end())
      throw TAccountInvalidCredentialsException();

    // Check if account is active.
    if (db_res[0][2].as<bool>() == false)
      throw TAccountDeactivatedException();

    // Check if password is correct.
    if (password != db_res[0][3].as<std::string>())
      throw TAccountInvalidCredentialsException();

    // Build account (standard mode).
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.active = true;
    _return.username = username;
    _return.first_name = db_res[0][4].as<std::string>();
    _return.last_name = db_res[0][5].as<std::string>();
  }

  void create_account(TAccount& _return,
      const TRequestMetadata& request_metadata, const std::string& username,
      const std::string& password, const std::string& first_name,
      const std::string& last_name) {
    // Validate attributes.
    if (!validate_attributes(username, password, first_name, last_name))
      throw TAccountInvalidAttributesException();

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "INSERT INTO Accounts (created_at, username, password, first_name, "
            "last_name) "
        "VALUES (extract(epoch from now()), '%s', '%s', '%s', '%s') "
        "RETURNING id, created_at";
    sprintf(query_str, query_fmt, username.c_str(), password.c_str(),
        first_name.c_str(), last_name.c_str());

    // Execute query.
    pqxx::result db_res;
    try {
      db_res = run_query(request_metadata, query_str, "account");
    }
    catch (pqxx::sql_error& e) {
      throw TAccountUsernameAlreadyExistsException();
    }

    // Build account (standard mode).
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.active = true;
    _return.username = username;
    _return.first_name = first_name;
    _return.last_name = last_name;
  }

  void retrieve_standard_account(TAccount& _return,
      const TRequestMetadata& request_metadata, int32_t account_id) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT created_at, active, username, first_name, last_name "
        "FROM Accounts "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, account_id);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "account");

    // Check if account exists.
    if (db_res.begin() == db_res.end())
      throw TAccountNotFoundException();

    // Build account (standard mode).
    _return.id = account_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.active = db_res[0][1].as<bool>();
    _return.username = db_res[0][2].as<std::string>();
    _return.first_name = db_res[0][3].as<std::string>();
    _return.last_name = db_res[0][4].as<std::string>();
  }

  void retrieve_expanded_account(TAccount& _return,
      const TRequestMetadata& request_metadata, int32_t account_id) {
    // Retrieve standard account.
    retrieve_standard_account(_return, request_metadata, account_id);

    // Retrieve follow activity in separate threads.
    auto follows_you_future = std::async(std::launch::async, [&] {
      return rpc_check_follow(request_metadata, account_id,
          request_metadata.requester_id);
    });
    auto followed_by_you_future = std::async(std::launch::async, [&] {
      return rpc_check_follow(request_metadata, request_metadata.requester_id,
          account_id);
    });
    auto n_followers_future = std::async(std::launch::async, [&] {
      return rpc_count_followers(request_metadata, account_id);
    });
    auto n_following_future = std::async(std::launch::async, [&] {
      return rpc_count_followees(request_metadata, account_id);
    });

    // Retrieve post activity in a separate thread.
    auto n_posts_future = std::async(std::launch::async, [&] {
      return rpc_count_posts_by_author(request_metadata, account_id);
    });

    // Retrieve like activity in a separate thread.
    auto n_likes_future = std::async(std::launch::async, [&] {
      return rpc_count_likes_by_account(request_metadata, account_id);
    });

    // Build account (expanded mode).
    _return.__set_follows_you(follows_you_future.get());
    _return.__set_followed_by_you(followed_by_you_future.get());
    _return.__set_n_followers(n_followers_future.get());
    _return.__set_n_following(n_following_future.get());
    _return.__set_n_posts(n_posts_future.get());
    _return.__set_n_likes(n_likes_future.get());
  }

  void update_account(TAccount& _return,
      const TRequestMetadata& request_metadata, const int32_t account_id,
      const std::string& password, const std::string& first_name,
      const std::string& last_name) {
    // Check if requester is authorized.
    if (request_metadata.requester_id != account_id)
      throw TAccountNotAuthorizedException();

    // Validate attributes.
    // NOTE: "john.doe" is a valid username. It is used because the actual
    // username is unknown and will not be updated anyway.
    if (!validate_attributes("john.doe", password, first_name, last_name))
      throw TAccountInvalidAttributesException();

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "UPDATE Accounts "
        "SET password = '%s', first_name = '%s', last_name = '%s' "
        "WHERE id = %d "
        "RETURNING created_at, active, username";
    sprintf(query_str, query_fmt, password.c_str(), first_name.c_str(),
        last_name.c_str(), account_id);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "account");

    // Check if account exists.
    if (db_res.begin() == db_res.end())
      throw TAccountNotFoundException();

    // Build account (standard mode).
    _return.id = account_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.active = db_res[0][1].as<bool>();
    _return.username = db_res[0][2].as<std::string>();
    _return.first_name = first_name;
    _return.last_name = last_name;
  }

  void delete_account(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    // Check if requester is authorized.
    if (request_metadata.requester_id != account_id)
      throw TAccountNotAuthorizedException();

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "UPDATE Accounts "
        "SET active = FALSE "
        "WHERE id = %d "
        "RETURNING id";
    sprintf(query_str, query_fmt, account_id);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "account");

    // Check if account exists.
    if (db_res.begin() == db_res.end())
      throw TAccountNotFoundException();
  }

  void list_accounts(std::vector<TAccount>& _return,
      const TRequestMetadata& request_metadata, const TAccountQuery& query,
      const int32_t limit, const int32_t offset) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at, active, username, first_name, last_name "
        "FROM Accounts "
        "WHERE %s "
        "ORDER BY created_at DESC "
        "LIMIT %d "
        "OFFSET %d";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str(), limit,
        offset);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "account");

    // Retrieve follow activity in separate threads.
    std::vector<std::future<bool>> follows_you_futures;
    for (auto row : db_res) {
      follows_you_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_check_follow(request_metadata, row["id"].as<int>(),
            request_metadata.requester_id);
      }));
    }
    std::vector<std::future<bool>> followed_by_you_futures;
    for (auto row : db_res) {
      followed_by_you_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_check_follow(request_metadata, request_metadata.requester_id,
            row["id"].as<int>());
      }));
    }
    std::vector<std::future<int>> n_followers_futures;
    for (auto row : db_res) {
      n_followers_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_count_followers(request_metadata, row["id"].as<int>());
      }));
    }
    std::vector<std::future<int>> n_following_futures;
    for (auto row : db_res) {
      n_following_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_count_followees(request_metadata, row["id"].as<int>());
      }));
    }

    // Retrieve post activity in separate threads.
    std::vector<std::future<int>> n_posts_futures;
    for (auto row : db_res) {
      n_posts_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_count_posts_by_author(request_metadata, row["id"].as<int>());
      }));
    }

    // Retrieve like activity in separate threads.
    std::vector<std::future<int>> n_likes_futures;
    for (auto row : db_res) {
      n_likes_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_count_likes_by_account(request_metadata, row["id"].as<int>());
      }));
    }

    // Build accounts.
    for (auto i = 0; i < db_res.size(); i++) {
      // Build account (expanded mode).
      TAccount account;
      account.id = db_res[i]["id"].as<int>();
      account.created_at = db_res[i]["created_at"].as<int>();
      account.active = db_res[i]["active"].as<bool>();
      account.username = db_res[i]["username"].as<std::string>();
      account.first_name = db_res[i]["first_name"].as<std::string>();
      account.last_name = db_res[i]["last_name"].as<std::string>();
      account.__set_follows_you(follows_you_futures[i].get());
      account.__set_followed_by_you(followed_by_you_futures[i].get());
      account.__set_n_followers(n_followers_futures[i].get());
      account.__set_n_following(n_following_futures[i].get());
      account.__set_n_posts(n_posts_futures[i].get());
      account.__set_n_likes(n_likes_futures[i].get());
      _return.push_back(account);
    }
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("account_server", "Account server");
  options.add_options()
      ("host", "", cxxopts::value<std::string>()->default_value("0.0.0.0"))
      ("port", "", cxxopts::value<int>())
      ("threads", "", cxxopts::value<int>()->default_value("0"))
      ("accept_backlog", "", cxxopts::value<int>()->default_value("0"))
      ("backend_filepath", "", cxxopts::value<std::string>()->default_value(
          "/etc/opt/BuzzBlog/backend.yml"))
      ("microservice_connection_pool_size", "", cxxopts::value<int>()->default_value(
          "0"))
      ("postgres_connection_pool_size", "", cxxopts::value<int>()->default_value(
          "0"))
      ("postgres_user", "", cxxopts::value<std::string>()->default_value(
          "postgres"))
      ("postgres_password", "", cxxopts::value<std::string>()->default_value(
          "postgres"));

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();
  int acceptBacklog = result["accept_backlog"].as<int>();
  std::string backend_filepath = result["backend_filepath"].as<std::string>();
  int microservice_connection_pool_size = result["microservice_connection_pool_size"].as<int>();
  int postgres_connection_pool_size = result["postgres_connection_pool_size"].as<int>();
  std::string postgres_user = result["postgres_user"].as<std::string>();
  std::string postgres_password = result["postgres_password"].as<std::string>();

  // Create server.
  auto socket = std::make_shared<TServerSocket>(host, port);
  if (acceptBacklog > 0)
    socket->setAcceptBacklog(acceptBacklog);
  TThreadedServer server(
      std::make_shared<TAccountServiceProcessor>(
          std::make_shared<TAccountServiceHandler>(backend_filepath,
              microservice_connection_pool_size, postgres_connection_pool_size,
              postgres_user, postgres_password)),
      socket,
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  if (threads > 0)
    server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
