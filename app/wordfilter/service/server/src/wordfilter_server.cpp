// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/gen/TWordfilterService.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <cxxopts.hpp>
#include <string>
#include <vector>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;

class TWordfilterServiceHandler : public TWordfilterServiceIf {
 public:
  TWordfilterServiceHandler(const int n_invalid_words, const int logging) {
    if (n_invalid_words > 0) invalid_words.push_back("corinthians");
    for (int i = 0; i < n_invalid_words - 1; i++)
      invalid_words.push_back(gen_random_string(11));
  }

  bool is_valid_word(const TRequestMetadata& request_metadata,
                     const std::string& word) {
    for (auto invalid_word : invalid_words)
      if (word == invalid_word) return false;
    return true;
  }

 private:
  std::string gen_random_string(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string random_string;
    for (int i = 0; i < len; i++)
      random_string += alphanum[rand() % (sizeof(alphanum) - 1)];
    return random_string;
  }

  std::vector<std::string> invalid_words;
};

int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("wordfilter_server", "Wordfilter server");
  options.add_options()(
      "host", "", cxxopts::value<std::string>()->default_value("0.0.0.0"))(
      "port", "",
      cxxopts::value<
          int>())("threads", "",
                  cxxopts::value<int>()->default_value(
                      "0"))("accept_backlog", "",
                            cxxopts::value<int>()->default_value(
                                "0"))("n_invalid_words", "",
                                      cxxopts::value<int>()->default_value(
                                          "0"))("logging", "",
                                                cxxopts::value<int>()
                                                    ->default_value("1"));

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();
  int acceptBacklog = result["accept_backlog"].as<int>();
  int n_invalid_words = result["n_invalid_words"].as<int>();
  int logging = result["logging"].as<int>();

  // Create server.
  auto socket = std::make_shared<TServerSocket>(host, port);
  if (acceptBacklog > 0) socket->setAcceptBacklog(acceptBacklog);
  TThreadedServer server(std::make_shared<TWordfilterServiceProcessor>(
                             std::make_shared<TWordfilterServiceHandler>(
                                 n_invalid_words, logging)),
                         socket, std::make_shared<TBufferedTransportFactory>(),
                         std::make_shared<TBinaryProtocolFactory>());
  if (threads > 0) server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
