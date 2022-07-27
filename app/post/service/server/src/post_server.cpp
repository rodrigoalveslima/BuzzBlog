// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <future>
#include <string>

#include <cxxopts.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TPostService.h>
#include <buzzblog/microservice_connected_server.h>
#include <buzzblog/postgres_connected_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;


class TPostServiceHandler :
    public MicroserviceConnectedServer,
    public PostgresConnectedServer,
    public TPostServiceIf {
private:
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
      const int microservice_connection_pool_size,
      const int postgres_connection_pool_size,
      const std::string& postgres_user, const std::string& postgres_password)
  : MicroserviceConnectedServer(backend_filepath, microservice_connection_pool_size),
    PostgresConnectedServer(backend_filepath, postgres_connection_pool_size, postgres_user, postgres_password) {
  }

  void create_post(TPost& _return, const TRequestMetadata& request_metadata,
      const std::string& text) {
    // Validate attributes.
    if (!validate_attributes(text))
      throw TPostInvalidAttributesException();

    // Update trending score of hashtags in a separate thread.
    auto trending_future = std::async(std::launch::async, [&] {
      return rpc_process_post(request_metadata, text);
    });

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "INSERT INTO Posts (text, author_id, created_at) "
        "VALUES ('%s', %d, extract(epoch from now())) "
        "RETURNING id, created_at";
    sprintf(query_str, query_fmt, text.c_str(), request_metadata.requester_id);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "post");

    // Build account (standard mode).
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.active = true;
    _return.text = text;
    _return.author_id = request_metadata.requester_id;

    // Sync trending thread.
    trending_future.get();
  }

  void retrieve_standard_post(TPost& _return,
      const TRequestMetadata& request_metadata, const int32_t post_id) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT created_at, active, text, author_id "
        "FROM Posts "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, post_id);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "post");

    // Check if post exists.
    if (db_res.begin() == db_res.end())
      throw TPostNotFoundException();

    // Build post (standard mode).
    _return.id = post_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.active = db_res[0][1].as<bool>();
    _return.text = db_res[0][2].as<std::string>();
    _return.author_id = db_res[0][3].as<int>();
  }

  void retrieve_expanded_post(TPost& _return,
      const TRequestMetadata& request_metadata, const int32_t post_id) {
    // Retrieve standard post.
    retrieve_standard_post(_return, request_metadata, post_id);

    // Retrieve author in a separate thread.
    auto author_future = std::async(std::launch::async, [&] {
      return rpc_retrieve_standard_account(request_metadata, _return.author_id);
    });

    // Retrieve like activity in a separate thread.
    auto n_likes_future = std::async(std::launch::async, [&] {
      return rpc_count_likes_of_post(request_metadata, post_id);
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
    const char *query_fmt = \
        "UPDATE Posts "
        "SET active = FALSE "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, post_id);

    // Execute query.
    run_query(request_metadata, query_str, "post");
  }

  void list_posts(std::vector<TPost>& _return,
      const TRequestMetadata& request_metadata, const TPostQuery& query,
      const int32_t limit, const int32_t offset) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at, active, text, author_id "
        "FROM Posts "
        "WHERE %s "
        "ORDER BY created_at DESC "
        "LIMIT %d "
        "OFFSET %d";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str(), limit,
        offset);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "post");

    // Retrieve authors in separate threads.
    std::vector<std::future<TAccount>> author_futures;
    for (auto row : db_res) {
      author_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_retrieve_standard_account(request_metadata,
            row["author_id"].as<int>());
      }));
    }

    // Retrieve like activity in separate threads.
    std::vector<std::future<int>> n_likes_futures;
    for (auto row : db_res) {
      n_likes_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_count_likes_of_post(request_metadata, row["id"].as<int>());
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
    const char *query_fmt = \
        "SELECT COUNT(*) "
        "FROM Posts "
        "WHERE author_id = %d";
    sprintf(query_str, query_fmt, author_id);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "post");

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
      std::make_shared<TPostServiceProcessor>(
          std::make_shared<TPostServiceHandler>(backend_filepath,
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
