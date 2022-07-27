// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef MICROSERVICE_CONNECTION_POOL__H
#define MICROSERVICE_CONNECTION_POOL__H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>


template<typename T>
class MicroserviceConnectionPool {
 private:
  std::vector<std::pair<std::string, int>> _servers;
  int _pool_size;
  int _pool_max_size;
  int _conn_timeout_ms;
  std::shared_ptr<spdlog::logger> _logger;
  std::queue<std::shared_ptr<T>> _conn_pool;
  std::mutex _conn_pool_mutex;
  std::condition_variable _conn_pool_condition;
 public:
  MicroserviceConnectionPool(const std::vector<std::pair<std::string, int>>& servers,
      const int pool_max_size, const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger) {
    _servers = servers;
    _pool_size = 0;
    _pool_max_size = pool_max_size;
    _conn_timeout_ms = conn_timeout_ms;
    _logger = logger;
  }

  ~MicroserviceConnectionPool() {
  }

  std::shared_ptr<T> get_client() {
    std::shared_ptr<T> conn;
    if (_pool_max_size == 0) {
      std::pair<std::string, int> server = _servers[rand() % int(_servers.size())];
      conn = std::make_shared<T>(server.first, server.second, _conn_timeout_ms, _logger);
    }
    else {
      std::unique_lock<std::mutex> lock(_conn_pool_mutex);
      if (_pool_size < _pool_max_size) {
        std::pair<std::string, int> server = _servers[_pool_size++ % int(_servers.size())];
        _conn_pool.emplace(std::make_shared<T>(server.first, server.second, _conn_timeout_ms, _logger));
      }
      while (_conn_pool.empty())
        _conn_pool_condition.wait(lock);
      conn = _conn_pool.front();
      _conn_pool.pop();
    }
    return conn;
  }

  void release_client(std::shared_ptr<T> conn) {
    if (_pool_max_size == 0) {
      conn->close();
    }
    else {
      std::unique_lock<std::mutex> lock(_conn_pool_mutex);
      _conn_pool.push(conn);
      lock.unlock();
      _conn_pool_condition.notify_one();
    }
  }
};

#endif
