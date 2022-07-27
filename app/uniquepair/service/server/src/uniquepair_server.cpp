// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <sstream>
#include <string>

#include <cxxopts.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TUniquepairService.h>
#include <buzzblog/postgres_connected_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;


class TUniquepairServiceHandler :
    public PostgresConnectedServer,
    public TUniquepairServiceIf {
private:
  std::string build_where_clause(const TUniquepairQuery& query) {
    std::ostringstream where_clause;
    where_clause << "domain = '" << query.domain << "'";
    if (query.__isset.first_elem)
      where_clause << " AND first_elem = " << query.first_elem;
    if (query.__isset.second_elem)
      where_clause << " AND second_elem = " << query.second_elem;
    return where_clause.str();
  }

public:
  TUniquepairServiceHandler(const std::string& backend_filepath,
      const int postgres_connection_pool_size,
      const std::string& postgres_user, const std::string& postgres_password)
  : PostgresConnectedServer(backend_filepath, postgres_connection_pool_size, postgres_user, postgres_password) {
  }

  void get(TUniquepair& _return, const TRequestMetadata& request_metadata,
      const int32_t uniquepair_id) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT created_at, domain, first_elem, second_elem "
        "FROM Uniquepairs "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, uniquepair_id);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "uniquepair");

    // Check if unique pair exists.
    if (db_res.begin() == db_res.end())
      throw TUniquepairNotFoundException();

    // Build unique pair.
    _return.id = uniquepair_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.domain = db_res[0][1].as<std::string>();
    _return.first_elem = db_res[0][2].as<int>();
    _return.second_elem = db_res[0][3].as<int>();
  }

  void add(TUniquepair& _return, const TRequestMetadata& request_metadata,
      const std::string& domain, const int32_t first_elem,
      const int32_t second_elem) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "INSERT INTO Uniquepairs (domain, first_elem, second_elem, created_at) "
        "VALUES ('%s', %d, %d, extract(epoch from now())) "
        "RETURNING id, created_at";
    sprintf(query_str, query_fmt, domain.c_str(), first_elem, second_elem);

    // Execute query.
    pqxx::result db_res;
    try {
      db_res = run_query(request_metadata, query_str, "uniquepair");
    }
    catch (pqxx::sql_error& e) {
      throw TUniquepairAlreadyExistsException();
    }

    // Build unique pair.
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.domain = domain;
    _return.first_elem = first_elem;
    _return.second_elem = second_elem;
  }

  void remove(const TRequestMetadata& request_metadata,
      const int32_t uniquepair_id) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "DELETE FROM Uniquepairs "
        "WHERE id = %d "
        "RETURNING id";
    sprintf(query_str, query_fmt, uniquepair_id);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "uniquepair");

    // Check if unique pair exists.
    if (db_res.begin() == db_res.end())
      throw TUniquepairNotFoundException();
  }

  void find(TUniquepair& _return, const TRequestMetadata& request_metadata,
      const std::string& domain, const int32_t first_elem,
      const int32_t second_elem) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at "
        "FROM Uniquepairs "
        "WHERE domain = '%s' AND first_elem = %d AND second_elem = %d";
    sprintf(query_str, query_fmt, domain.c_str(), first_elem, second_elem);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "uniquepair");

    // Check if unique pair exists.
    if (db_res.begin() == db_res.end())
      throw TUniquepairNotFoundException();

    // Build unique pair.
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.domain = domain;
    _return.first_elem = first_elem;
    _return.second_elem = second_elem;
  }

  void fetch(std::vector<TUniquepair>& _return,
      const TRequestMetadata& request_metadata, const TUniquepairQuery& query,
      const int32_t limit, const int32_t offset) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at, first_elem, second_elem "
        "FROM Uniquepairs "
        "WHERE %s "
        "ORDER BY created_at DESC "
        "LIMIT %d "
        "OFFSET %d";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str(), limit,
        offset);

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "uniquepair");

    // Build unique pairs.
    for (auto row : db_res) {
      // Build unique pair.
      TUniquepair uniquepair;
      uniquepair.id = row["id"].as<int>();
      uniquepair.created_at = row["created_at"].as<int>();
      uniquepair.domain = query.domain;
      uniquepair.first_elem = row["first_elem"].as<int>();
      uniquepair.second_elem = row["second_elem"].as<int>();
      _return.push_back(uniquepair);
    }
  }

  int32_t count(const TRequestMetadata& request_metadata,
      const TUniquepairQuery& query) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT COUNT(*) "
        "FROM Uniquepairs "
        "WHERE %s";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str());

    // Execute query.
    auto db_res = run_query(request_metadata, query_str, "uniquepair");

    return db_res[0][0].as<int>();
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("uniquepair_server", "Uniquepair server");
  options.add_options()
      ("host", "", cxxopts::value<std::string>()->default_value("0.0.0.0"))
      ("port", "", cxxopts::value<int>())
      ("threads", "", cxxopts::value<int>()->default_value("0"))
      ("accept_backlog", "", cxxopts::value<int>()->default_value("0"))
      ("backend_filepath", "", cxxopts::value<std::string>()->default_value(
          "/etc/opt/BuzzBlog/backend.yml"))
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
  int postgres_connection_pool_size = result["postgres_connection_pool_size"].as<int>();
  std::string postgres_user = result["postgres_user"].as<std::string>();
  std::string postgres_password = result["postgres_password"].as<std::string>();

  // Create server.
  auto socket = std::make_shared<TServerSocket>(host, port);
  if (acceptBacklog > 0)
    socket->setAcceptBacklog(acceptBacklog);
  TThreadedServer server(
      std::make_shared<TUniquepairServiceProcessor>(
          std::make_shared<TUniquepairServiceHandler>(backend_filepath,
              postgres_connection_pool_size, postgres_user, postgres_password)),
      socket,
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  if (threads > 0)
    server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
