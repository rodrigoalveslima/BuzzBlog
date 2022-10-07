// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/gen/TTrendingService.h>
#include <buzzblog/microservice_connected_server.h>
#include <buzzblog/redis_connected_server.h>
#include <buzzblog/utils.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <cxxopts.hpp>
#include <sstream>
#include <string>
#include <vector>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;

class TTrendingServiceHandler : public MicroserviceConnectedServer,
                                public RedisConnectedServer,
                                public TTrendingServiceIf {
 private:
  std::shared_ptr<spdlog::logger> _rpc_logger;
  std::shared_ptr<spdlog::logger> _redis_logger;

 public:
  TTrendingServiceHandler(
      const std::string& backend_filepath,
      const int microservice_connection_pool_min_size,
      const int microservice_connection_pool_max_size,
      const int microservice_connection_pool_allow_ephemeral,
      const int redis_connection_pool_size, const int logging)
      : MicroserviceConnectedServer(
            "trending", backend_filepath, microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral != 0, logging),
        RedisConnectedServer(backend_filepath, redis_connection_pool_size) {
    if (logging) {
      _rpc_logger = spdlog::basic_logger_mt("rpc_logger", "/tmp/rpc.log");
      _rpc_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
      _redis_logger = spdlog::basic_logger_mt("redis_logger", "/tmp/redis.log");
      _redis_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
    } else {
      _rpc_logger = nullptr;
      _redis_logger = nullptr;
    }
  }

  void process_post(const TRequestMetadata& request_metadata,
                    const std::string& text) {
    std::istringstream text_iss(text);
    do {
      std::string word;
      text_iss >> word;
      if (word.size() > 1 && word[0] == '#') {
        auto is_valid_word = RPC_WRAPPER<bool>(
            std::bind(&TTrendingServiceHandler::rpc_is_valid_word, this,
                      std::ref(request_metadata), word.substr(1)),
            _rpc_logger,
            "ls=trending lf=process_post rs=wordfilter rf=is_valid_word rid=" +
                request_metadata.id);
        if (is_valid_word)
          VOID_RPC_WRAPPER(
              std::bind(&TTrendingServiceHandler::zincrby, this,
                        std::ref(request_metadata), "trending", "hashtags", 1,
                        word.substr(1)),
              _redis_logger,
              "ls=trending lf=process_post key=trending cm=zincrby rid=" +
                  request_metadata.id);
      }
    } while (text_iss);
  }

  void fetch_trending_hashtags(std::vector<std::string>& _return,
                               const TRequestMetadata& request_metadata,
                               const int32_t limit) {
    _return = RPC_WRAPPER<std::vector<std::string>>(
        std::bind(&TTrendingServiceHandler::zrange<std::vector<std::string>>,
                  this, std::ref(request_metadata), "trending", "hashtags", 0,
                  std::ref(limit)),
        _redis_logger,
        "ls=trending lf=fetch_trending_hashtags key=trending cm=zrange rid=" +
            request_metadata.id);
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
      ("backend_filepath", "",
          cxxopts::value<std::string>()->default_value("/etc/opt/BuzzBlog/backend.yml"))
      ("microservice_connection_pool_min_size", "",
          cxxopts::value<int>()->default_value("0"))
      ("microservice_connection_pool_max_size", "",
          cxxopts::value<int>()->default_value("0"))
      ("microservice_connection_pool_allow_ephemeral", "",
          cxxopts::value<int>()->default_value("0"))
      ("redis_connection_pool_size", "",
          cxxopts::value<int>()->default_value("0"))
      ("logging", "", cxxopts::value<int>()->default_value("1"));

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();
  int acceptBacklog = result["accept_backlog"].as<int>();
  std::string backend_filepath = result["backend_filepath"].as<std::string>();
  int microservice_connection_pool_min_size =
      result["microservice_connection_pool_min_size"].as<int>();
  int microservice_connection_pool_max_size =
      result["microservice_connection_pool_max_size"].as<int>();
  int microservice_connection_pool_allow_ephemeral =
      result["microservice_connection_pool_allow_ephemeral"].as<int>();
  int redis_connection_pool_size =
      result["redis_connection_pool_size"].as<int>();
  int logging = result["logging"].as<int>();

  // Create server.
  auto socket = std::make_shared<TServerSocket>(host, port);
  if (acceptBacklog > 0) socket->setAcceptBacklog(acceptBacklog);
  TThreadedServer server(
      std::make_shared<TTrendingServiceProcessor>(
          std::make_shared<TTrendingServiceHandler>(
              backend_filepath, microservice_connection_pool_min_size,
              microservice_connection_pool_max_size,
              microservice_connection_pool_allow_ephemeral,
              redis_connection_pool_size, logging)),
      socket, std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  if (threads > 0) server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
