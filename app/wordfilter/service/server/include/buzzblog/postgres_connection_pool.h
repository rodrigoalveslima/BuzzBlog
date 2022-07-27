// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef POSTGRES_CONNECTION_POOL__H
#define POSTGRES_CONNECTION_POOL__H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include <pqxx/pqxx>
#include <spdlog/sinks/basic_file_sink.h>

class PostgresConnectionPool {
 private:
  std::string _conn_cstr;
  int _pool_size;
  int _pool_max_size;
  std::queue<std::shared_ptr<pqxx::connection>> _conn_pool;
  std::mutex _conn_pool_mutex;
  std::condition_variable _conn_pool_condition;
 public:
  PostgresConnectionPool(const std::string& conn_cstr, const int pool_max_size) {
    _conn_cstr = conn_cstr;
    _pool_size = 0;
    _pool_max_size = pool_max_size;
  }

  ~PostgresConnectionPool() {
  }

  std::shared_ptr<pqxx::connection> get_client() {
    std::shared_ptr<pqxx::connection> conn;
    if (_pool_max_size == 0) {
      conn = std::make_shared<pqxx::connection>(_conn_cstr);
    }
    else {
      std::unique_lock<std::mutex> lock(_conn_pool_mutex);
      if (_pool_size < _pool_max_size) {
        _conn_pool.emplace(std::make_shared<pqxx::connection>(_conn_cstr));
        _pool_size++;
      }
      while (_conn_pool.empty())
        _conn_pool_condition.wait(lock);
      conn = _conn_pool.front();
      _conn_pool.pop();
    }
    return conn;
  }

  void release_client(std::shared_ptr<pqxx::connection> conn) {
    if (_pool_max_size == 0) {
      conn->disconnect();
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
