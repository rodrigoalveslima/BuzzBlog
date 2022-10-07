// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef POSTGRES_CONNECTED_SERVER__H
#define POSTGRES_CONNECTED_SERVER__H

#include <buzzblog/base_server.h>
#include <buzzblog/gen/buzzblog_types.h>
#include <buzzblog/postgres_connection_pool.h>
#include <buzzblog/utils.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <yaml-cpp/yaml.h>

#include <map>
#include <memory>
#include <string>

class PostgresConnectedServer : public BaseServer {
 protected:
  PostgresConnectedServer(const std::string& local_service_name,
                          const std::string& backend_filepath,
                          const int postgres_connection_pool_min_size,
                          const int postgres_connection_pool_max_size,
                          const bool postgres_connection_pool_allow_ephemeral,
                          const std::string& postgres_user,
                          const std::string& postgres_password,
                          const int logging) {
    _local_service_name = local_service_name;
    // Set PostgreSQL connection string format.
    char conn_cstr[128];
    const char* conn_fmt = "postgres://%s:%s@%s:%d/%s";

    // Parse backend configuration.
    stdout_log("Initializing PostgresConnectedServer");
    auto backend_conf = YAML::LoadFile(backend_filepath);

    // Initialize loggers.
    std::shared_ptr<spdlog::logger> query_conn_logger;
    if (logging) {
      _query_call_logger =
          spdlog::basic_logger_mt("query_call_logger", "/tmp/query_call.log");
      _query_call_logger->set_pattern(
          "[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
      query_conn_logger =
          spdlog::basic_logger_mt("query_conn_logger", "/tmp/query_conn.log");
      query_conn_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
    } else {
      _query_call_logger = nullptr;
      query_conn_logger = nullptr;
    }

    // Process backend configuration.
    for (const auto& it : backend_conf) {
      auto service_name = it.first.as<std::string>();
      auto service_conf = it.second;
      // Process service database configuration.
      if (service_conf["database"]) {
        auto db_address = service_conf["database"].as<std::string>();
        auto db_host = db_address.substr(0, db_address.find(":"));
        auto db_port = std::stoi(db_address.substr(db_address.find(":") + 1));
        sprintf(conn_cstr, conn_fmt, postgres_user.c_str(),
                postgres_password.c_str(), db_host.c_str(), db_port,
                service_name.c_str());
        _cp[service_name] = std::make_shared<PostgresConnectionPool>(
            local_service_name, service_name, std::string(conn_cstr),
            postgres_connection_pool_min_size,
            postgres_connection_pool_max_size,
            postgres_connection_pool_allow_ephemeral, query_conn_logger);
        stdout_log("Added " + service_name + " database on: " + db_address);
      }
    }
  }

  pqxx::result run_query(const std::string& query, const std::string& dbname) {
    pqxx::result res;
    auto conn = _cp[dbname]->get_client();
    try {
      VOID_RPC_WRAPPER(
          std::bind(&PostgresConnectedServer::exec_and_commit, this,
                    std::ref(res), std::ref(query), std::ref(conn)),
          _query_call_logger, "db=" + dbname + " ls=" + _local_service_name);
    } catch (...) {
      _cp[dbname]->release_client(conn);
      throw;
    }
    _cp[dbname]->release_client(conn);
    return res;
  }

 private:
  std::string _local_service_name;
  std::shared_ptr<spdlog::logger> _query_call_logger;
  // Database connection pools.
  std::map<std::string, std::shared_ptr<PostgresConnectionPool>> _cp;

  void exec_and_commit(pqxx::result& res, const std::string& query,
                       const std::shared_ptr<pqxx::connection> conn) {
    pqxx::work txn(*conn);
    res = txn.exec(query);
    txn.commit();
  }
};

#endif
