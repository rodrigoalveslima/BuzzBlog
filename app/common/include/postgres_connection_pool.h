// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef POSTGRES_CONNECTION_POOL__H
#define POSTGRES_CONNECTION_POOL__H

#include <assert.h>
#include <buzzblog/gen/buzzblog_types.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <pqxx/pqxx>
#include <queue>
#include <string>

class PostgresConnectionPool {
 private:
  std::string _local_service_name;
  std::string _dbname;
  std::string _conn_cstr;
  int _pool_current_size;
  int _pool_min_size;
  int _pool_max_size;
  int _backlog_len;
  bool _allow_ephemeral;
  std::queue<std::shared_ptr<pqxx::connection>> _conn_pool;
  std::mutex _conn_pool_mutex;
  std::condition_variable _conn_pool_condition;
  std::shared_ptr<spdlog::logger> _query_conn_logger;

 public:
  PostgresConnectionPool(const std::string& local_service_name,
                         const std::string& dbname,
                         const std::string& conn_cstr, const int pool_min_size,
                         const int pool_max_size, const bool allow_ephemeral,
                         std::shared_ptr<spdlog::logger> query_conn_logger) {
    _local_service_name = local_service_name;
    _dbname = dbname;
    _conn_cstr = conn_cstr;
    _pool_min_size = pool_min_size;
    _pool_max_size = pool_max_size;
    _allow_ephemeral = allow_ephemeral;
    _query_conn_logger = query_conn_logger;
    _pool_current_size = 0;
    _backlog_len = 0;

    // Validate connection pool parameters.
    assert(_pool_min_size >= 0);
    assert(_pool_max_size >= 0);
    assert(_pool_max_size >= _pool_min_size);
  }

  ~PostgresConnectionPool() {}

  std::shared_ptr<pqxx::connection> get_client() {
    auto start_time = std::chrono::steady_clock::now();
    int backlog_len = 0;
    std::shared_ptr<pqxx::connection> conn = nullptr;
    if (_pool_max_size > 0) {
      std::unique_lock<std::mutex> lock(_conn_pool_mutex);
      if (_pool_current_size < _pool_min_size) {
        _pool_current_size++;
      } else if (_conn_pool.size() > 0) {
        conn = _conn_pool.front();
        _conn_pool.pop();
      } else if (_pool_current_size < _pool_max_size || _allow_ephemeral) {
        _pool_current_size++;
      } else {
        backlog_len = ++_backlog_len;
        while (_conn_pool.empty()) _conn_pool_condition.wait(lock);
        _backlog_len--;
        conn = _conn_pool.front();
        _conn_pool.pop();
      }
      lock.unlock();
    }
    if (conn == nullptr) conn = std::make_shared<pqxx::connection>(_conn_cstr);
    std::chrono::duration<double> latency =
        std::chrono::steady_clock::now() - start_time;
    if (_query_conn_logger)
      _query_conn_logger->info("ls={} db={} bl={} lat={}", _local_service_name,
                               _dbname, backlog_len, latency.count());
    return conn;
  }

  void release_client(std::shared_ptr<pqxx::connection> conn) {
    if (_pool_max_size > 0) {
      std::unique_lock<std::mutex> lock(_conn_pool_mutex);
      if (_pool_current_size > _pool_max_size ||
          (_pool_current_size > _pool_min_size && _conn_pool.size() > 1)) {
        conn->disconnect();
        _pool_current_size--;
      } else {
        _conn_pool.push(conn);
        _conn_pool_condition.notify_one();
      }
      lock.unlock();
    } else {
      conn->disconnect();
    }
  }
};

#endif
