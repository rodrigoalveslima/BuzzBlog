// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef POSTGRES_CONNECTED_SERVER__H
#define POSTGRES_CONNECTED_SERVER__H

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include <spdlog/sinks/basic_file_sink.h>
#include <yaml-cpp/yaml.h>

#include <buzzblog/gen/buzzblog_types.h>
#include <buzzblog/base_server.h>
#include <buzzblog/postgres_connection_pool.h>

class PostgresConnectedServer :
    public BaseServer {
 protected:
  PostgresConnectedServer(const std::string& backend_filepath,
      const int postgres_connection_pool_size,
      const std::string& postgres_user,
      const std::string& postgres_password) {
    // Set PostgreSQL connection string format.
    char conn_cstr[128];
    const char *conn_fmt = "postgres://%s:%s@%s:%d/%s";

    // Initialize logger.
    query_logger = spdlog::basic_logger_mt("query_logger", "/tmp/queries.log");
    query_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");

    // Parse backend configuration.
    stdout_log("Initializing PostgresConnectedServer");
    auto backend_conf = YAML::LoadFile(backend_filepath);

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
        _cp[service_name] = std::make_shared<PostgresConnectionPool>(std::string(conn_cstr),
            postgres_connection_pool_size);
        stdout_log("Added " + service_name + " database on: " + db_address);
      }
    }
  }

  pqxx::result run_query(const gen::TRequestMetadata& request_metadata,
      const std::string& query, const std::string& dbname) {
    auto start_time = std::chrono::steady_clock::now();
    pqxx::result res;
    auto conn = _cp[dbname]->get_client();
    try {
      pqxx::work txn(*conn);
      res = txn.exec(query);
      txn.commit();
    }
    catch (...) {
      _cp[dbname]->release_client(conn);
      throw;
    }
    _cp[dbname]->release_client(conn);
    std::chrono::duration<double> latency = std::chrono::steady_clock::now() - start_time;
    query_logger->info("request_id={} latency={} query=\"{}\"",
        request_metadata.id, latency.count(), query);
    return res;
  }

 private:
  // Logger.
  std::shared_ptr<spdlog::logger> query_logger;

  // Database connection pools.
  std::map<std::string, std::shared_ptr<PostgresConnectionPool>> _cp;
};

#endif
