// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef BASE_CLIENT__H
#define BASE_CLIENT__H

#include <chrono>
#include <memory>
#include <string>

#include <spdlog/sinks/basic_file_sink.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


template<typename T>
class BaseClient {
 private:
  std::string _ip_address;
  int _port;
  std::shared_ptr<spdlog::logger> _logger;
  std::shared_ptr<TSocket> _socket;
  std::shared_ptr<TTransport> _transport;
  std::shared_ptr<TProtocol> _protocol;
 public:
  std::shared_ptr<T> _client;

  BaseClient(const std::string& ip_address, const int port,
      const int conn_timeout_ms, std::shared_ptr<spdlog::logger> logger) {
    _ip_address = ip_address;
    _port = port;
    _logger = logger;
    _socket = std::make_shared<TSocket>(ip_address, port);
    _socket->setConnTimeout(conn_timeout_ms);
    _transport = std::make_shared<TBufferedTransport>(_socket);
    _protocol = std::make_shared<TBinaryProtocol>(_transport);
    _client = std::make_shared<T>(_protocol);
    _transport->open();
  }

  ~BaseClient() {
    close();
  }

  void close() {
    if (_transport->isOpen())
      _transport->close();
  }

  template<typename U>
  U RPC_WRAPPER(std::function<U()> rpc, const std::string& request_id,
      const std::string& function_name) {
    auto start_time = std::chrono::steady_clock::now();
    U ret = rpc();
    std::chrono::duration<double> latency = std::chrono::steady_clock::now() - \
        start_time;
    _logger->info("request_id={} server={}:{} function={} latency={}",
        request_id, _ip_address, _port, function_name, latency.count());
    return ret;
  }

  void VOID_RPC_WRAPPER(std::function<void()> rpc,
      const std::string& request_id, const std::string& function_name) {
    auto start_time = std::chrono::steady_clock::now();
    rpc();
    std::chrono::duration<double> latency = std::chrono::steady_clock::now() - \
        start_time;
    _logger->info("request_id={} server={}:{} function={} latency={}",
        request_id, _ip_address, _port, function_name, latency.count());
  }
};

#endif
