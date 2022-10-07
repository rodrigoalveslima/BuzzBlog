// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/gen/TPostService.h>
#include <buzzblog/microservice_connected_server.h>
#include <buzzblog/postgres_connected_server.h>
#include <buzzblog/utils.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <cxxopts.hpp>
#include <future>
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;

class TPostServiceHandler : public MicroserviceConnectedServer,
                            public PostgresConnectedServer,
                            public TPostServiceIf {
 private:
  std::shared_ptr<spdlog::logger> _rpc_logger;
  std::shared_ptr<spdlog::logger> _query_logger;

  bool validate_attributes(const std::string& text) {
    return (text.size() > 0 && text.size() <= 200);
  }

  std::string build_where_clause(const TPostQuery& query) {
    std::ostringstream where_clause;
    where_clause << "active = true";
    if (query.__isset.author_id)
      where_clause << " AND author_id = " << query.author_id;
    return where_clause.str();
  }

 public:
  TPostServiceHandler(const std::string& backend_filepath,
                      const int microservice_connection_pool_min_size,
                      const int microservice_connection_pool_max_size,
                      const int microservice_connection_pool_allow_ephemeral,
                      const int postgres_connection_pool_min_size,
                      const int postgres_connection_pool_max_size,
                      const int postgres_connection_pool_allow_ephemeral,
                      const std::string& postgres_user,
                      const std::string& postgres_password, const int logging)
      : MicroserviceConnectedServer(
            "post", backend_filepath, microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral != 0, logging),
        PostgresConnectedServer("post", backend_filepath,
                                postgres_connection_pool_min_size,
                                postgres_connection_pool_max_size,
                                postgres_connection_pool_allow_ephemeral != 0,
                                postgres_user, postgres_password, logging) {
    if (logging) {
      _rpc_logger = spdlog::basic_logger_mt("rpc_logger", "/tmp/rpc.log");
      _rpc_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
      _query_logger = spdlog::basic_logger_mt("query_logger", "/tmp/query.log");
      _query_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
    } else {
      _rpc_logger = nullptr;
      _query_logger = nullptr;
    }
  }

  void create_post(TPost& _return, const TRequestMetadata& request_metadata,
                   const std::string& text) {
    // Validate attributes.
    if (!validate_attributes(text)) throw TPostInvalidAttributesException();

    // Update trending score of hashtags in a separate thread.
    auto trending_future = std::async(std::launch::async, [&] {
      return VOID_RPC_WRAPPER(
          std::bind(&TPostServiceHandler::rpc_process_post, this,
                    std::ref(request_metadata), std::ref(text)),
          _rpc_logger,
          "ls=post lf=create_post rs=trending rf=process_post rid=" +
              request_metadata.id);
    });

    // Build query string.
    char query_str[1024];
    const char* query_fmt =
        "INSERT INTO Posts (text, author_id, created_at) "
        "VALUES ('%s', %d, extract(epoch from now())) "
        "RETURNING id, created_at";
    sprintf(query_str, query_fmt, text.c_str(), request_metadata.requester_id);

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TPostServiceHandler::run_query, this, std::ref(query_str),
                  "post"),
        _query_logger,
        "ls=post lf=create_post db=post qt=insert rid=" + request_metadata.id);

    // Build post (standard mode).
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.active = true;
    _return.text = text;
    _return.author_id = request_metadata.requester_id;

    // Sync trending thread.
    trending_future.get();
  }

  void retrieve_standard_post(TPost& _return,
                              const TRequestMetadata& request_metadata,
                              const int32_t post_id) {
    // Build query string.
    char query_str[1024];
    const char* query_fmt =
        "SELECT created_at, active, text, author_id "
        "FROM Posts "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, post_id);

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TPostServiceHandler::run_query, this, std::ref(query_str),
                  "post"),
        _query_logger,
        "ls=post lf=retrieve_standard_post db=post qt=select rid=" +
            request_metadata.id);

    // Check if post exists.
    if (db_res.begin() == db_res.end()) throw TPostNotFoundException();

    // Build post (standard mode).
    _return.id = post_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.active = db_res[0][1].as<bool>();
    _return.text = db_res[0][2].as<std::string>();
    _return.author_id = db_res[0][3].as<int>();
  }

  void retrieve_expanded_post(TPost& _return,
                              const TRequestMetadata& request_metadata,
                              const int32_t post_id) {
    // Retrieve standard post.
    retrieve_standard_post(_return, request_metadata, post_id);

    // Retrieve author in a separate thread.
    auto author_future = std::async(std::launch::async, [&] {
      return RPC_WRAPPER<TAccount>(
          std::bind(&TPostServiceHandler::rpc_retrieve_standard_account, this,
                    std::ref(request_metadata), std::ref(_return.author_id)),
          _rpc_logger,
          "ls=post lf=retrieve_expanded_post rs=account "
          "rf=retrieve_standard_account rid=" +
              request_metadata.id);
    });

    // Retrieve like activity in a separate thread.
    auto n_likes_future = std::async(std::launch::async, [&] {
      return RPC_WRAPPER<int32_t>(
          std::bind(&TPostServiceHandler::rpc_count_likes_of_post, this,
                    std::ref(request_metadata), std::ref(post_id)),
          _rpc_logger,
          "ls=post lf=retrieve_expanded_post rs=like rf=count_likes_of_post "
          "rid=" +
              request_metadata.id);
    });

    // Build post (expanded mode).
    _return.__set_author(author_future.get());
    _return.__set_n_likes(n_likes_future.get());
  }

  void delete_post(const TRequestMetadata& request_metadata,
                   const int32_t post_id) {
    {
      // Retrieve standard post.
      TPost post;
      retrieve_standard_post(post, request_metadata, post_id);

      // Check if requester is authorized.
      if (request_metadata.requester_id != post.author_id)
        throw TPostNotAuthorizedException();
    }

    // Build query string.
    char query_str[1024];
    const char* query_fmt =
        "UPDATE Posts "
        "SET active = FALSE "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, post_id);

    // Execute query.
    RPC_WRAPPER<pqxx::result>(
        std::bind(&TPostServiceHandler::run_query, this, std::ref(query_str),
                  "post"),
        _query_logger,
        "ls=post lf=delete_post db=post qt=update rid=" + request_metadata.id);
  }

  void list_posts(std::vector<TPost>& _return,
                  const TRequestMetadata& request_metadata,
                  const TPostQuery& query, const int32_t limit,
                  const int32_t offset) {
    // Build query string.
    char query_str[1024];
    const char* query_fmt =
        "SELECT id, created_at, active, text, author_id "
        "FROM Posts "
        "WHERE %s "
        "ORDER BY created_at DESC "
        "LIMIT %d "
        "OFFSET %d";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str(), limit,
            offset);

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TPostServiceHandler::run_query, this, std::ref(query_str),
                  "post"),
        _query_logger,
        "ls=post lf=list_posts db=post qt=select rid=" + request_metadata.id);

    // Retrieve authors in separate threads.
    std::vector<std::future<TAccount>> author_futures;
    for (auto row : db_res) {
      author_futures.push_back(std::async(std::launch::async, [&] {
        return RPC_WRAPPER<TAccount>(
            std::bind(&TPostServiceHandler::rpc_retrieve_standard_account, this,
                      std::ref(request_metadata), row["author_id"].as<int>()),
            _rpc_logger,
            "ls=post lf=list_posts rs=account rf=retrieve_standard_account "
            "rid=" +
                request_metadata.id);
      }));
    }

    // Retrieve like activity in separate threads.
    std::vector<std::future<int>> n_likes_futures;
    for (auto row : db_res) {
      n_likes_futures.push_back(std::async(std::launch::async, [&] {
        return RPC_WRAPPER<int32_t>(
            std::bind(&TPostServiceHandler::rpc_count_likes_of_post, this,
                      std::ref(request_metadata), row["id"].as<int>()),
            _rpc_logger,
            "ls=post lf=list_posts rs=like rf=count_likes_of_post rid=" +
                request_metadata.id);
      }));
    }

    // Build posts.
    for (auto i = 0; i < db_res.size(); i++) {
      // Build post (expanded mode).
      TPost post;
      post.id = db_res[i]["id"].as<int>();
      post.created_at = db_res[i]["created_at"].as<int>();
      post.active = db_res[i]["active"].as<bool>();
      post.text = db_res[i]["text"].as<std::string>();
      post.author_id = db_res[i]["author_id"].as<int>();
      post.__set_author(author_futures[i].get());
      post.__set_n_likes(n_likes_futures[i].get());
      _return.push_back(post);
    }
  }

  int32_t count_posts_by_author(const TRequestMetadata& request_metadata,
                                const int32_t author_id) {
    // Build query string.
    char query_str[1024];
    const char* query_fmt =
        "SELECT COUNT(*) "
        "FROM Posts "
        "WHERE author_id = %d";
    sprintf(query_str, query_fmt, author_id);

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TPostServiceHandler::run_query, this, std::ref(query_str),
                  "post"),
        _query_logger,
        "ls=post lf=count_posts_by_author db=post qt=select rid=" +
            request_metadata.id);

    return db_res[0][0].as<int>();
  }
};

int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("post_server", "Post server");
  options.add_options()
      ("host", "", cxxopts::value<std::string>()->default_value("0.0.0.0"))
      ("port", "", cxxopts::value<int>())
      ("threads", "", cxxopts::value<int>()->default_value("0"))
      ("accept_backlog", "", cxxopts::value<int>()->default_value("0"))
      ("backend_filepath", "",
          cxxopts::value<std::string>()->default_value("/etc/opt/BuzzBlog/backend.yml"))
      ("microservice_connection_pool_min_size", "",
          cxxopts::value<int>()->default_value("0"))
      ("microservice_connection_pool_max_size", "",
          cxxopts::value<int>()->default_value("0"))
      ("microservice_connection_pool_allow_ephemeral", "",
          cxxopts::value<int>()->default_value("0"))
      ("postgres_connection_pool_min_size", "",
          cxxopts::value<int>()->default_value("0"))
      ("postgres_connection_pool_max_size", "",
          cxxopts::value<int>()->default_value("0"))
      ("postgres_connection_pool_allow_ephemeral", "",
          cxxopts::value<int>()->default_value("0"))
      ("postgres_user", "",
          cxxopts::value<std::string>()->default_value("postgres"))
      ("postgres_password", "",
          cxxopts::value<std::string>()->default_value("postgres"))
      ("logging", "", cxxopts::value<int>()->default_value("1"));

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();
  int acceptBacklog = result["accept_backlog"].as<int>();
  std::string backend_filepath = result["backend_filepath"].as<std::string>();
  int microservice_connection_pool_min_size =
      result["microservice_connection_pool_min_size"].as<int>();
  int microservice_connection_pool_max_size =
      result["microservice_connection_pool_max_size"].as<int>();
  int microservice_connection_pool_allow_ephemeral =
      result["microservice_connection_pool_allow_ephemeral"].as<int>();
  int postgres_connection_pool_min_size =
      result["postgres_connection_pool_min_size"].as<int>();
  int postgres_connection_pool_max_size =
      result["postgres_connection_pool_max_size"].as<int>();
  int postgres_connection_pool_allow_ephemeral =
      result["postgres_connection_pool_allow_ephemeral"].as<int>();
  std::string postgres_user = result["postgres_user"].as<std::string>();
  std::string postgres_password = result["postgres_password"].as<std::string>();
  int logging = result["logging"].as<int>();

  // Create server.
  auto socket = std::make_shared<TServerSocket>(host, port);
  if (acceptBacklog > 0) socket->setAcceptBacklog(acceptBacklog);
  TThreadedServer server(
      std::make_shared<TPostServiceProcessor>(
          std::make_shared<TPostServiceHandler>(
              backend_filepath, microservice_connection_pool_min_size,
              microservice_connection_pool_max_size,
              microservice_connection_pool_allow_ephemeral,
              postgres_connection_pool_min_size,
              postgres_connection_pool_max_size,
              postgres_connection_pool_allow_ephemeral, postgres_user,
              postgres_password, logging)),
      socket, std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  if (threads > 0) server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
