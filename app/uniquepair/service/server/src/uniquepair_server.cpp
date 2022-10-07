// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/gen/TUniquepairService.h>
#include <buzzblog/postgres_connected_server.h>
#include <buzzblog/utils.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <cxxopts.hpp>
#include <sstream>
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;

class TUniquepairServiceHandler : public PostgresConnectedServer,
                                  public TUniquepairServiceIf {
 private:
  std::shared_ptr<spdlog::logger> _query_logger;

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
                            const int postgres_connection_pool_min_size,
                            const int postgres_connection_pool_max_size,
                            const int postgres_connection_pool_allow_ephemeral,
                            const std::string& postgres_user,
                            const std::string& postgres_password,
                            const int logging)
      : PostgresConnectedServer("uniquepair", backend_filepath,
                                postgres_connection_pool_min_size,
                                postgres_connection_pool_max_size,
                                postgres_connection_pool_allow_ephemeral != 0,
                                postgres_user, postgres_password, logging) {
    if (logging) {
      _query_logger = spdlog::basic_logger_mt("query_logger", "/tmp/query.log");
      _query_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
    } else {
      _query_logger = nullptr;
    }
  }

  void get(TUniquepair& _return, const TRequestMetadata& request_metadata,
           const int32_t uniquepair_id) {
    // Build query string.
    char query_str[1024];
    const char* query_fmt =
        "SELECT created_at, domain, first_elem, second_elem "
        "FROM Uniquepairs "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, uniquepair_id);

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TUniquepairServiceHandler::run_query, this,
                  std::ref(query_str), "uniquepair"),
        _query_logger,
        "ls=uniquepair lf=get db=uniquepair qt=select rid=" +
            request_metadata.id);

    // Check if unique pair exists.
    if (db_res.begin() == db_res.end()) throw TUniquepairNotFoundException();

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
    const char* query_fmt =
        "INSERT INTO Uniquepairs (domain, first_elem, second_elem, created_at) "
        "VALUES ('%s', %d, %d, extract(epoch from now())) "
        "RETURNING id, created_at";
    sprintf(query_str, query_fmt, domain.c_str(), first_elem, second_elem);

    // Execute query.
    pqxx::result db_res;
    try {
      db_res = RPC_WRAPPER<pqxx::result>(
          std::bind(&TUniquepairServiceHandler::run_query, this,
                    std::ref(query_str), "uniquepair"),
          _query_logger,
          "ls=uniquepair lf=add db=uniquepair qt=insert rid=" +
              request_metadata.id);
    } catch (pqxx::sql_error& e) {
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
    const char* query_fmt =
        "DELETE FROM Uniquepairs "
        "WHERE id = %d "
        "RETURNING id";
    sprintf(query_str, query_fmt, uniquepair_id);

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TUniquepairServiceHandler::run_query, this,
                  std::ref(query_str), "uniquepair"),
        _query_logger,
        "ls=uniquepair lf=remove db=uniquepair qt=delete rid=" +
            request_metadata.id);

    // Check if unique pair exists.
    if (db_res.begin() == db_res.end()) throw TUniquepairNotFoundException();
  }

  bool find(const TRequestMetadata& request_metadata, const std::string& domain,
            const int32_t first_elem, const int32_t second_elem) {
    // Build query string.
    char query_str[1024];
    const char* query_fmt =
        "SELECT id, created_at "
        "FROM Uniquepairs "
        "WHERE domain = '%s' AND first_elem = %d AND second_elem = %d";
    sprintf(query_str, query_fmt, domain.c_str(), first_elem, second_elem);

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TUniquepairServiceHandler::run_query, this,
                  std::ref(query_str), "uniquepair"),
        _query_logger,
        "ls=uniquepair lf=find db=uniquepair qt=select rid=" +
            request_metadata.id);

    // Check if unique pair was found.
    return (db_res.begin() != db_res.end());
  }

  void fetch(std::vector<TUniquepair>& _return,
             const TRequestMetadata& request_metadata,
             const TUniquepairQuery& query, const int32_t limit,
             const int32_t offset) {
    // Build query string.
    char query_str[1024];
    const char* query_fmt =
        "SELECT id, created_at, first_elem, second_elem "
        "FROM Uniquepairs "
        "WHERE %s "
        "ORDER BY created_at DESC "
        "LIMIT %d "
        "OFFSET %d";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str(), limit,
            offset);

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TUniquepairServiceHandler::run_query, this,
                  std::ref(query_str), "uniquepair"),
        _query_logger,
        "ls=uniquepair lf=fetch db=uniquepair qt=select rid=" +
            request_metadata.id);

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
    const char* query_fmt =
        "SELECT COUNT(*) "
        "FROM Uniquepairs "
        "WHERE %s";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str());

    // Execute query.
    auto db_res = RPC_WRAPPER<pqxx::result>(
        std::bind(&TUniquepairServiceHandler::run_query, this,
                  std::ref(query_str), "uniquepair"),
        _query_logger,
        "ls=uniquepair lf=count db=uniquepair qt=select rid=" +
            request_metadata.id);

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
      ("backend_filepath", "",
          cxxopts::value<std::string>()->default_value("/etc/opt/BuzzBlog/backend.yml"))
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
      std::make_shared<TUniquepairServiceProcessor>(
          std::make_shared<TUniquepairServiceHandler>(
              backend_filepath, postgres_connection_pool_min_size,
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
