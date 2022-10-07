// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/gen/TLikeService.h>
#include <buzzblog/microservice_connected_server.h>
#include <buzzblog/utils.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <cxxopts.hpp>
#include <future>
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;

class TLikeServiceHandler : public MicroserviceConnectedServer,
                            public TLikeServiceIf {
 private:
  std::shared_ptr<spdlog::logger> _rpc_logger;

 public:
  TLikeServiceHandler(const std::string& backend_filepath,
                      const int microservice_connection_pool_min_size,
                      const int microservice_connection_pool_max_size,
                      const int microservice_connection_pool_allow_ephemeral,
                      const int logging)
      : MicroserviceConnectedServer(
            "like", backend_filepath, microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral != 0, logging) {
    if (logging) {
      _rpc_logger = spdlog::basic_logger_mt("rpc_logger", "/tmp/rpc.log");
      _rpc_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
    } else {
      _rpc_logger = nullptr;
    }
  }

  void like_post(TLike& _return, const TRequestMetadata& request_metadata,
                 const int32_t post_id) {
    // Add unique pair (account, post).
    TUniquepair uniquepair;
    try {
      uniquepair = RPC_WRAPPER<TUniquepair>(
          std::bind(&TLikeServiceHandler::rpc_add, this,
                    std::ref(request_metadata), "like",
                    std::ref(request_metadata.requester_id), std::ref(post_id)),
          _rpc_logger,
          "ls=like lf=like_post rs=uniquepair rf=add rid=" +
              request_metadata.id);
    } catch (TUniquepairAlreadyExistsException e) {
      throw TLikeAlreadyExistsException();
    }

    // Build like (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.account_id = request_metadata.requester_id;
    _return.post_id = post_id;
  }

  void retrieve_standard_like(TLike& _return,
                              const TRequestMetadata& request_metadata,
                              const int32_t like_id) {
    // Get unique pair.
    TUniquepair uniquepair;
    try {
      uniquepair = RPC_WRAPPER<TUniquepair>(
          std::bind(&TLikeServiceHandler::rpc_get, this,
                    std::ref(request_metadata), std::ref(like_id)),
          _rpc_logger,
          "ls=like lf=retrieve_standard_like rs=uniquepair rf=get rid=" +
              request_metadata.id);
    } catch (TUniquepairNotFoundException e) {
      throw TLikeNotFoundException();
    }

    // Build like (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.account_id = uniquepair.first_elem;
    _return.post_id = uniquepair.second_elem;
  }

  void retrieve_expanded_like(TLike& _return,
                              const TRequestMetadata& request_metadata,
                              const int32_t like_id) {
    // Retrieve standard like.
    retrieve_standard_like(_return, request_metadata, like_id);

    // Retrieve account in a separate thread.
    auto account_future = std::async(std::launch::async, [&] {
      return RPC_WRAPPER<TAccount>(
          std::bind(&TLikeServiceHandler::rpc_retrieve_standard_account, this,
                    std::ref(request_metadata), std::ref(_return.account_id)),
          _rpc_logger,
          "ls=like lf=retrieve_expanded_like rs=account "
          "rf=retrieve_standard_account rid=" +
              request_metadata.id);
    });

    // Retrieve post in a separate thread.
    auto post_future = std::async(std::launch::async, [&] {
      return RPC_WRAPPER<TPost>(
          std::bind(&TLikeServiceHandler::rpc_retrieve_expanded_post, this,
                    std::ref(request_metadata), std::ref(_return.post_id)),
          _rpc_logger,
          "ls=like lf=retrieve_expanded_like rs=post rf=retrieve_expanded_post "
          "rid=" +
              request_metadata.id);
    });

    // Build like (expanded mode).
    _return.__set_account(account_future.get());
    _return.__set_post(post_future.get());
  }

  void delete_like(const TRequestMetadata& request_metadata,
                   const int32_t like_id) {
    {
      // Get unique pair.
      TUniquepair uniquepair;
      try {
        uniquepair = RPC_WRAPPER<TUniquepair>(
            std::bind(&TLikeServiceHandler::rpc_get, this,
                      std::ref(request_metadata), std::ref(like_id)),
            _rpc_logger,
            "ls=like lf=delete_like rs=uniquepair rf=get rid=" +
                request_metadata.id);
      } catch (TUniquepairNotFoundException e) {
        throw TLikeNotFoundException();
      }

      // Check if requester is authorized.
      if (request_metadata.requester_id != uniquepair.first_elem)
        throw TLikeNotAuthorizedException();
    }

    // Remove unique pair.
    try {
      VOID_RPC_WRAPPER(std::bind(&TLikeServiceHandler::rpc_remove, this,
                                 std::ref(request_metadata), std::ref(like_id)),
                       _rpc_logger,
                       "ls=like lf=delete_like rs=uniquepair rf=remove rid=" +
                           request_metadata.id);
    } catch (TUniquepairNotFoundException e) {
      throw TLikeNotFoundException();
    }
  }

  void list_likes(std::vector<TLike>& _return,
                  const TRequestMetadata& request_metadata,
                  const TLikeQuery& query, const int32_t limit,
                  const int32_t offset) {
    // Build query struct.
    TUniquepairQuery uniquepair_query;
    uniquepair_query.__set_domain("like");
    if (query.__isset.account_id)
      uniquepair_query.__set_first_elem(query.account_id);
    if (query.__isset.post_id)
      uniquepair_query.__set_second_elem(query.post_id);

    // Fetch unique pairs.
    auto uniquepairs = RPC_WRAPPER<std::vector<TUniquepair>>(
        std::bind(&TLikeServiceHandler::rpc_fetch, this,
                  std::ref(request_metadata), std::ref(uniquepair_query),
                  std::ref(limit), std::ref(offset)),
        _rpc_logger,
        "ls=like lf=list_likes rs=uniquepair rf=fetch rid=" +
            request_metadata.id);

    // Retrieve accounts in separate threads.
    std::vector<std::future<TAccount>> account_futures;
    for (auto it : uniquepairs) {
      account_futures.push_back(std::async(std::launch::async, [&] {
        return RPC_WRAPPER<TAccount>(
            std::bind(&TLikeServiceHandler::rpc_retrieve_standard_account, this,
                      std::ref(request_metadata), std::ref(it.first_elem)),
            _rpc_logger,
            "ls=like lf=list_likes rs=account rf=retrieve_standard_account "
            "rid=" +
                request_metadata.id);
      }));
    }

    // Retrieve posts in separate threads.
    std::vector<std::future<TPost>> post_futures;
    for (auto it : uniquepairs) {
      post_futures.push_back(std::async(std::launch::async, [&] {
        return RPC_WRAPPER<TPost>(
            std::bind(&TLikeServiceHandler::rpc_retrieve_expanded_post, this,
                      std::ref(request_metadata), std::ref(it.second_elem)),
            _rpc_logger,
            "ls=like lf=list_likes rs=post rf=retrieve_expanded_post rid=" +
                request_metadata.id);
      }));
    }

    // Build likes.
    for (auto i = 0; i < uniquepairs.size(); i++) {
      auto uniquepair = uniquepairs[i];
      auto account = account_futures[i].get();
      auto post = post_futures[i].get();
      // Build like (expanded mode).
      TLike like;
      like.id = uniquepair.id;
      like.created_at = uniquepair.created_at;
      like.account_id = uniquepair.first_elem;
      like.post_id = uniquepair.second_elem;
      like.__set_account(account);
      like.__set_post(post);
      _return.push_back(like);
    }
  }

  int32_t count_likes_by_account(const TRequestMetadata& request_metadata,
                                 const int32_t account_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("like");
    query.__set_first_elem(account_id);

    // Count unique pairs.
    return RPC_WRAPPER<int32_t>(
        std::bind(&TLikeServiceHandler::rpc_count, this,
                  std::ref(request_metadata), std::ref(query)),
        _rpc_logger,
        "ls=like lf=count_likes_by_account rs=uniquepair rf=count rid=" +
            request_metadata.id);
  }

  int32_t count_likes_of_post(const TRequestMetadata& request_metadata,
                              const int32_t post_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("like");
    query.__set_second_elem(post_id);

    // Count unique pairs.
    return RPC_WRAPPER<int32_t>(
        std::bind(&TLikeServiceHandler::rpc_count, this,
                  std::ref(request_metadata), std::ref(query)),
        _rpc_logger,
        "ls=like lf=count_likes_of_post rs=uniquepair rf=count rid=" +
            request_metadata.id);
  }
};

int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("like_server", "Like server");
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
  int logging = result["logging"].as<int>();

  // Create server.
  auto socket = std::make_shared<TServerSocket>(host, port);
  if (acceptBacklog > 0) socket->setAcceptBacklog(acceptBacklog);
  TThreadedServer server(
      std::make_shared<TLikeServiceProcessor>(
          std::make_shared<TLikeServiceHandler>(
              backend_filepath, microservice_connection_pool_min_size,
              microservice_connection_pool_max_size,
              microservice_connection_pool_allow_ephemeral, logging)),
      socket, std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  if (threads > 0) server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
