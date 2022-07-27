// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <sstream>
#include <string>
#include <vector>

#include <cxxopts.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TTrendingService.h>
#include <buzzblog/microservice_connected_server.h>
#include <buzzblog/redis_connected_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;


class TTrendingServiceHandler :
    public MicroserviceConnectedServer,
    public RedisConnectedServer,
    public TTrendingServiceIf {
public:
  TTrendingServiceHandler(const std::string& backend_filepath,
      const int microservice_connection_pool_size,
      const int redis_connection_pool_size)
  : MicroserviceConnectedServer(backend_filepath, microservice_connection_pool_size),
    RedisConnectedServer(backend_filepath, redis_connection_pool_size) {
  }

  void process_post(const TRequestMetadata& request_metadata,
      const std::string& text) {
    std::istringstream text_iss(text);
    do {
      std::string word;
      text_iss >> word;
      if (word.size() > 1 && word[0] == '#' &&
          rpc_is_valid_word(request_metadata, word.substr(1)))
        zincrby(request_metadata, "trending", "hashtags", 1, word.substr(1));
    } while (text_iss);
  }

  void fetch_trending_hashtags(std::vector<std::string>& _return,
      const TRequestMetadata& request_metadata, const int32_t limit) {
    _return = zrange<std::vector<std::string>>(request_metadata, "trending",
        "hashtags", 0, limit);
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("trending_server", "Trending server");
  options.add_options()
      ("host", "", cxxopts::value<std::string>()->default_value("0.0.0.0"))
      ("port", "", cxxopts::value<int>())
      ("threads", "", cxxopts::value<int>()->default_value("0"))
      ("accept_backlog", "", cxxopts::value<int>()->default_value("0"))
      ("backend_filepath", "", cxxopts::value<std::string>()->default_value(
          "/etc/opt/BuzzBlog/backend.yml"))
      ("microservice_connection_pool_size", "", cxxopts::value<int>()->default_value(
          "0"))
      ("redis_connection_pool_size", "", cxxopts::value<int>()->default_value(
          "0"));

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();
  int acceptBacklog = result["accept_backlog"].as<int>();
  std::string backend_filepath = result["backend_filepath"].as<std::string>();
  int microservice_connection_pool_size = result["microservice_connection_pool_size"].as<int>();
  int redis_connection_pool_size = result["redis_connection_pool_size"].as<int>();

  // Create server.
  auto socket = std::make_shared<TServerSocket>(host, port);
  if (acceptBacklog > 0)
    socket->setAcceptBacklog(acceptBacklog);
  TThreadedServer server(
      std::make_shared<TTrendingServiceProcessor>(
          std::make_shared<TTrendingServiceHandler>(backend_filepath,
              microservice_connection_pool_size, redis_connection_pool_size)),
      socket,
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  if (threads > 0)
    server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
