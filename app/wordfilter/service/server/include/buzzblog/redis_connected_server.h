// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef REDIS_CONNECTED_SERVER__H
#define REDIS_CONNECTED_SERVER__H

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include <spdlog/sinks/basic_file_sink.h>
#include <sw/redis++/redis++.h>
#include <yaml-cpp/yaml.h>

#include <buzzblog/gen/buzzblog_types.h>
#include <buzzblog/base_server.h>

using namespace sw::redis;

class RedisConnectedServer :
    public BaseServer {
 protected:
  RedisConnectedServer(const std::string& backend_filepath,
      const int redis_connection_pool_size) {
    // Initialize logger.
    redis_logger = spdlog::basic_logger_mt("redis_logger", "/tmp/redis.log");
    redis_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");

    // Parse backend configuration.
    stdout_log("Initializing RedisConnectedServer");
    auto backend_conf = YAML::LoadFile(backend_filepath);

    // Process backend configuration.
    for (const auto& it : backend_conf) {
      auto service_name = it.first.as<std::string>();
      auto service_conf = it.second;
      // Process service database configuration.
      if (service_conf["redis"]) {
        auto redis_address = service_conf["redis"].as<std::string>();
        auto redis_host = redis_address.substr(0, redis_address.find(":"));
        auto redis_port = std::stoi(redis_address.substr(redis_address.find(":") + 1));
        ConnectionOptions connection_options;
        connection_options.host = redis_host;
        connection_options.port = redis_port;
        ConnectionPoolOptions pool_options;
        pool_options.size = redis_connection_pool_size;
        redis_conn_pool[service_name] = std::make_shared<Redis>(connection_options, pool_options);
        stdout_log("Added " + service_name + " Redis server on: " + redis_address);
      }
    }
  }

  template<typename T>
  T zrange(const gen::TRequestMetadata& request_metadata,
      const std::string& service_name,
      const std::string& key, const int start, const int stop) {
    T members;
    auto start_time = std::chrono::steady_clock::now();
    redis_conn_pool[service_name]->zrange(key, start, stop,
        std::back_inserter(members));
    std::chrono::duration<double> latency = std::chrono::steady_clock::now() - \
        start_time;
    redis_logger->info("request_id={} latency={} service_name={} command={}",
        request_metadata.id, latency.count(), service_name, "zrange");
    return members;
  }

  double zincrby(const gen::TRequestMetadata& request_metadata,
      const std::string& service_name,
      const std::string& key, double increment, const std::string& member) {
    auto start_time = std::chrono::steady_clock::now();
    double score = redis_conn_pool[service_name]->zincrby(key, increment, member);
    std::chrono::duration<double> latency = std::chrono::steady_clock::now() - \
        start_time;
    redis_logger->info("request_id={} latency={} service_name={} command={}",
        request_metadata.id, latency.count(), service_name, "zincrby");
    return score;
  }

 private:
  // Logger.
  std::shared_ptr<spdlog::logger> redis_logger;
  // Redis connection pools.
  std::map<std::string, std::shared_ptr<Redis>> redis_conn_pool;
};

#endif
