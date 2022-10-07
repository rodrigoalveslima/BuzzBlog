// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <buzzblog/gen/TFollowService.h>
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

class TFollowServiceHandler : public MicroserviceConnectedServer,
                              public TFollowServiceIf {
 private:
  std::shared_ptr<spdlog::logger> _rpc_logger;

 public:
  TFollowServiceHandler(const std::string& backend_filepath,
                        const int microservice_connection_pool_min_size,
                        const int microservice_connection_pool_max_size,
                        const int microservice_connection_pool_allow_ephemeral,
                        const int logging)
      : MicroserviceConnectedServer(
            "follow", backend_filepath, microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral != 0, logging) {
    if (logging) {
      _rpc_logger = spdlog::basic_logger_mt("rpc_logger", "/tmp/rpc.log");
      _rpc_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
    } else {
      _rpc_logger = nullptr;
    }
  }

  void follow_account(TFollow& _return,
                      const TRequestMetadata& request_metadata,
                      const int32_t account_id) {
    // Validate attributes.
    if (request_metadata.requester_id == account_id)
      throw TFollowInvalidAttributesException();

    // Add unique pair (follower, followee).
    TUniquepair uniquepair;
    try {
      uniquepair = RPC_WRAPPER<TUniquepair>(
          std::bind(&TFollowServiceHandler::rpc_add, this,
                    std::ref(request_metadata), "follow",
                    std::ref(request_metadata.requester_id),
                    std::ref(account_id)),
          _rpc_logger,
          "ls=follow lf=follow_account rs=uniquepair rf=add rid=" +
              request_metadata.id);
    } catch (TUniquepairAlreadyExistsException e) {
      throw TFollowAlreadyExistsException();
    }

    // Build follow (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.follower_id = request_metadata.requester_id;
    _return.followee_id = account_id;
  }

  void retrieve_standard_follow(TFollow& _return,
                                const TRequestMetadata& request_metadata,
                                const int32_t follow_id) {
    // Get unique pair.
    TUniquepair uniquepair;
    try {
      uniquepair = RPC_WRAPPER<TUniquepair>(
          std::bind(&TFollowServiceHandler::rpc_get, this,
                    std::ref(request_metadata), std::ref(follow_id)),
          _rpc_logger,
          "ls=follow lf=retrieve_standard_follow rs=uniquepair rf=get rid=" +
              request_metadata.id);
    } catch (TUniquepairNotFoundException e) {
      throw TFollowNotFoundException();
    }

    // Build follow (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.follower_id = uniquepair.first_elem;
    _return.followee_id = uniquepair.second_elem;
  }

  void retrieve_expanded_follow(TFollow& _return,
                                const TRequestMetadata& request_metadata,
                                const int32_t follow_id) {
    // Retrieve standard follow.
    retrieve_standard_follow(_return, request_metadata, follow_id);

    // Retrieve follower in a separate thread.
    auto follower_future = std::async(std::launch::async, [&] {
      return RPC_WRAPPER<TAccount>(
          std::bind(&TFollowServiceHandler::rpc_retrieve_standard_account, this,
                    std::ref(request_metadata), std::ref(_return.follower_id)),
          _rpc_logger,
          "ls=follow lf=retrieve_expanded_follow rs=account "
          "rf=retrieve_standard_account rid=" +
              request_metadata.id);
    });

    // Retrieve followee in a separate thread.
    auto followee_future = std::async(std::launch::async, [&] {
      return RPC_WRAPPER<TAccount>(
          std::bind(&TFollowServiceHandler::rpc_retrieve_standard_account, this,
                    std::ref(request_metadata), std::ref(_return.followee_id)),
          _rpc_logger,
          "ls=follow lf=retrieve_expanded_follow rs=account "
          "rf=retrieve_standard_account rid=" +
              request_metadata.id);
    });

    // Build follow (expanded mode).
    _return.__set_follower(follower_future.get());
    _return.__set_followee(followee_future.get());
  }

  void delete_follow(const TRequestMetadata& request_metadata,
                     const int32_t follow_id) {
    {
      // Get unique pair.
      TUniquepair uniquepair;
      try {
        uniquepair = RPC_WRAPPER<TUniquepair>(
            std::bind(&TFollowServiceHandler::rpc_get, this,
                      std::ref(request_metadata), std::ref(follow_id)),
            _rpc_logger,
            "ls=follow lf=delete_follow rs=uniquepair rf=get rid=" +
                request_metadata.id);
      } catch (TUniquepairNotFoundException e) {
        throw TFollowNotFoundException();
      }

      // Check if requester is authorized.
      if (request_metadata.requester_id != uniquepair.first_elem)
        throw TFollowNotAuthorizedException();
    }

    // Remove unique pair.
    try {
      VOID_RPC_WRAPPER(
          std::bind(&TFollowServiceHandler::rpc_remove, this,
                    std::ref(request_metadata), std::ref(follow_id)),
          _rpc_logger,
          "ls=follow lf=delete_follow rs=uniquepair rf=remove rid=" +
              request_metadata.id);
    } catch (TUniquepairNotFoundException e) {
      throw TFollowNotFoundException();
    }
  }

  void list_follows(std::vector<TFollow>& _return,
                    const TRequestMetadata& request_metadata,
                    const TFollowQuery& query, const int32_t limit,
                    const int32_t offset) {
    // Build query struct.
    TUniquepairQuery uniquepair_query;
    uniquepair_query.__set_domain("follow");
    if (query.__isset.follower_id)
      uniquepair_query.__set_first_elem(query.follower_id);
    if (query.__isset.followee_id)
      uniquepair_query.__set_second_elem(query.followee_id);

    // Fetch unique pairs.
    auto uniquepairs = RPC_WRAPPER<std::vector<TUniquepair>>(
        std::bind(&TFollowServiceHandler::rpc_fetch, this,
                  std::ref(request_metadata), std::ref(uniquepair_query),
                  std::ref(limit), std::ref(offset)),
        _rpc_logger,
        "ls=follow lf=list_follows rs=uniquepair rf=fetch rid=" +
            request_metadata.id);

    // Retrieve followers in separate threads.
    std::vector<std::future<TAccount>> follower_futures;
    for (auto it : uniquepairs) {
      follower_futures.push_back(std::async(std::launch::async, [&] {
        return RPC_WRAPPER<TAccount>(
            std::bind(&TFollowServiceHandler::rpc_retrieve_standard_account,
                      this, std::ref(request_metadata),
                      std::ref(it.first_elem)),
            _rpc_logger,
            "ls=follow lf=list_follows rs=account rf=retrieve_standard_account "
            "rid=" +
                request_metadata.id);
      }));
    }

    // Retrieve followees in separate threads.
    std::vector<std::future<TAccount>> followee_futures;
    for (auto it : uniquepairs) {
      followee_futures.push_back(std::async(std::launch::async, [&] {
        return RPC_WRAPPER<TAccount>(
            std::bind(&TFollowServiceHandler::rpc_retrieve_standard_account,
                      this, std::ref(request_metadata),
                      std::ref(it.second_elem)),
            _rpc_logger,
            "ls=follow lf=list_follows rs=account rf=retrieve_standard_account "
            "rid=" +
                request_metadata.id);
      }));
    }

    // Build follows.
    for (auto i = 0; i < uniquepairs.size(); i++) {
      auto uniquepair = uniquepairs[i];
      auto follower = follower_futures[i].get();
      auto followee = followee_futures[i].get();
      // Build follow (expanded mode).
      TFollow follow;
      follow.id = uniquepair.id;
      follow.created_at = uniquepair.created_at;
      follow.follower_id = uniquepair.first_elem;
      follow.followee_id = uniquepair.second_elem;
      follow.__set_follower(follower);
      follow.__set_followee(followee);
      _return.push_back(follow);
    }
  }

  bool check_follow(const TRequestMetadata& request_metadata,
                    const int32_t follower_id, const int32_t followee_id) {
    return RPC_WRAPPER<bool>(
        std::bind(&TFollowServiceHandler::rpc_find, this,
                  std::ref(request_metadata), "follow", std::ref(follower_id),
                  std::ref(followee_id)),
        _rpc_logger,
        "ls=follow lf=check_follow rs=uniquepair rf=find rid=" +
            request_metadata.id);
  }

  int32_t count_followers(const TRequestMetadata& request_metadata,
                          const int32_t account_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("follow");
    query.__set_second_elem(account_id);

    // Count unique pairs.
    return RPC_WRAPPER<int32_t>(
        std::bind(&TFollowServiceHandler::rpc_count, this,
                  std::ref(request_metadata), std::ref(query)),
        _rpc_logger,
        "ls=follow lf=count_followers rs=uniquepair rf=count rid=" +
            request_metadata.id);
  }

  int32_t count_followees(const TRequestMetadata& request_metadata,
                          const int32_t account_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("follow");
    query.__set_first_elem(account_id);

    // Count unique pairs.
    return RPC_WRAPPER<int32_t>(
        std::bind(&TFollowServiceHandler::rpc_count, this,
                  std::ref(request_metadata), std::ref(query)),
        _rpc_logger,
        "ls=follow lf=count_followees rs=uniquepair rf=count rid=" +
            request_metadata.id);
  }
};

int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("follow_server", "Follow server");
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
      std::make_shared<TFollowServiceProcessor>(
          std::make_shared<TFollowServiceHandler>(
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
