// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef MICROSERVICE_CONNECTION_POOL__H
#define MICROSERVICE_CONNECTION_POOL__H

#include <assert.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

template <typename T>
class MicroserviceConnectionPool {
 private:
  std::string _local_service_name;
  std::string _remote_service_name;
  std::vector<std::pair<std::string, int>> _servers;
  int _pool_current_size;
  int _pool_min_size;
  int _pool_max_size;
  int _backlog_len;
  int _conn_timeout_ms;
  bool _allow_ephemeral;
  std::queue<std::shared_ptr<T>> _conn_pool;
  std::mutex _conn_pool_mutex;
  std::condition_variable _conn_pool_condition;
  std::shared_ptr<spdlog::logger> _rpc_conn_logger;

 public:
  MicroserviceConnectionPool(
      const std::string& local_service_name,
      const std::string& remote_service_name,
      const std::vector<std::pair<std::string, int>>& servers,
      const int pool_min_size, const int pool_max_size,
      const bool allow_ephemeral, const int conn_timeout_ms,
      std::shared_ptr<spdlog::logger> rpc_conn_logger) {
    _local_service_name = local_service_name;
    _remote_service_name = remote_service_name;
    _servers = servers;
    _pool_min_size = pool_min_size;
    _pool_max_size = pool_max_size;
    _allow_ephemeral = allow_ephemeral;
    _conn_timeout_ms = conn_timeout_ms;
    _rpc_conn_logger = rpc_conn_logger;
    _pool_current_size = 0;
    _backlog_len = 0;

    // Validate connection pool parameters.
    assert(_pool_min_size >= 0);
    assert(_pool_max_size >= 0);
    assert(_pool_max_size >= _pool_min_size);
  }

  ~MicroserviceConnectionPool() {}

  std::shared_ptr<T> get_client() {
    auto start_time = std::chrono::steady_clock::now();
    int backlog_len = 0;
    std::shared_ptr<T> conn = nullptr;
    std::pair<std::string, int> server;
    if (_pool_max_size > 0) {
      std::unique_lock<std::mutex> lock(_conn_pool_mutex);
      if (_pool_current_size < _pool_min_size) {
        server = _servers[_pool_current_size++ % int(_servers.size())];
        conn =
            std::make_shared<T>(server.first, server.second, _conn_timeout_ms);
      } else if (_conn_pool.size() > 0) {
        conn = _conn_pool.front();
        _conn_pool.pop();
      } else if (_pool_current_size < _pool_max_size || _allow_ephemeral) {
        server = _servers[_pool_current_size++ % int(_servers.size())];
        conn =
            std::make_shared<T>(server.first, server.second, _conn_timeout_ms);
      } else {
        backlog_len = ++_backlog_len;
        while (_conn_pool.empty()) _conn_pool_condition.wait(lock);
        _backlog_len--;
        conn = _conn_pool.front();
        _conn_pool.pop();
      }
      lock.unlock();
    } else {
      server = _servers[rand() % int(_servers.size())];
      conn = std::make_shared<T>(server.first, server.second, _conn_timeout_ms);
    }
    std::chrono::duration<double> latency =
        std::chrono::steady_clock::now() - start_time;
    if (_rpc_conn_logger)
      _rpc_conn_logger->info("ls={} rs={} bl={} lat={}", _local_service_name,
                             _remote_service_name, backlog_len,
                             latency.count());
    return conn;
  }

  void release_client(std::shared_ptr<T> conn) {
    if (_pool_max_size > 0) {
      std::unique_lock<std::mutex> lock(_conn_pool_mutex);
      if (_pool_current_size > _pool_max_size ||
          (_pool_current_size > _pool_min_size && _conn_pool.size() > 1)) {
        conn->close();
        _pool_current_size--;
      } else {
        _conn_pool.push(conn);
        _conn_pool_condition.notify_one();
      }
      lock.unlock();
    } else {
      conn->close();
    }
  }
};

#endif
