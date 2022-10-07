// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef REDIS_CONNECTED_SERVER__H
#define REDIS_CONNECTED_SERVER__H

#include <buzzblog/base_server.h>
#include <buzzblog/gen/buzzblog_types.h>
#include <sw/redis++/redis++.h>
#include <yaml-cpp/yaml.h>

#include <iostream>
#include <memory>
#include <string>

using namespace sw::redis;

class RedisConnectedServer : public BaseServer {
 protected:
  RedisConnectedServer(const std::string& backend_filepath,
                       const int redis_connection_pool_size) {
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
        auto redis_port =
            std::stoi(redis_address.substr(redis_address.find(":") + 1));
        ConnectionOptions connection_options;
        connection_options.host = redis_host;
        connection_options.port = redis_port;
        ConnectionPoolOptions pool_options;
        pool_options.size = redis_connection_pool_size;
        _cp[service_name] =
            std::make_shared<Redis>(connection_options, pool_options);
        stdout_log("Added " + service_name +
                   " Redis server on: " + redis_address);
      }
    }
  }

  template <typename T>
  T zrange(const gen::TRequestMetadata& request_metadata,
           const std::string& service_name, const std::string& key,
           const int start, const int stop) {
    T members;
    _cp[service_name]->zrange(key, start, stop, std::back_inserter(members));
    return members;
  }

  double zincrby(const gen::TRequestMetadata& request_metadata,
                 const std::string& service_name, const std::string& key,
                 double increment, const std::string& member) {
    return _cp[service_name]->zincrby(key, increment, member);
  }

 private:
  // Redis connection pools.
  std::map<std::string, std::shared_ptr<Redis>> _cp;
};

#endif
