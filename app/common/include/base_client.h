// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef BASE_CLIENT__H
#define BASE_CLIENT__H

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <memory>
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

template <typename T>
class BaseClient {
 private:
  std::string _ip_address;
  int _port;
  std::shared_ptr<TSocket> _socket;
  std::shared_ptr<TTransport> _transport;
  std::shared_ptr<TProtocol> _protocol;

 public:
  std::shared_ptr<T> _client;

  BaseClient(const std::string& ip_address, const int port,
             const int conn_timeout_ms) {
    _ip_address = ip_address;
    _port = port;
    _socket = std::make_shared<TSocket>(ip_address, port);
    _socket->setConnTimeout(conn_timeout_ms);
    _transport = std::make_shared<TBufferedTransport>(_socket);
    _protocol = std::make_shared<TBinaryProtocol>(_transport);
    _client = std::make_shared<T>(_protocol);
    _transport->open();
  }

  ~BaseClient() { close(); }

  void close() {
    if (_transport->isOpen()) _transport->close();
  }
};

#endif
