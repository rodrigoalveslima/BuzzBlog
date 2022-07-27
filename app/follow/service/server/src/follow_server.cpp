// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <future>
#include <string>

#include <cxxopts.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TFollowService.h>
#include <buzzblog/microservice_connected_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;


class TFollowServiceHandler :
    public MicroserviceConnectedServer,
    public TFollowServiceIf {
public:
  TFollowServiceHandler(const std::string& backend_filepath,
      const int microservice_connection_pool_size)
  : MicroserviceConnectedServer(backend_filepath, microservice_connection_pool_size) {
  }

  void follow_account(TFollow& _return,
      const TRequestMetadata& request_metadata, const int32_t account_id) {
    // Add unique pair (follower, followee).
    TUniquepair uniquepair;
    try {
      uniquepair = rpc_add(request_metadata, "follow", request_metadata.requester_id,
          account_id);
    }
    catch (TUniquepairAlreadyExistsException e) {
      throw TFollowAlreadyExistsException();
    }

    // Build follow (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.follower_id = request_metadata.requester_id;
    _return.followee_id = account_id;
  }

  void retrieve_standard_follow(TFollow& _return,
      const TRequestMetadata& request_metadata, const int32_t follow_id) {
    // Get unique pair.
    TUniquepair uniquepair;
    try {
      uniquepair = rpc_get(request_metadata, follow_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TFollowNotFoundException();
    }

    // Build follow (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.follower_id = uniquepair.first_elem;
    _return.followee_id = uniquepair.second_elem;
  }

  void retrieve_expanded_follow(TFollow& _return,
      const TRequestMetadata& request_metadata, const int32_t follow_id) {
    // Retrieve standard follow.
    retrieve_standard_follow(_return, request_metadata, follow_id);

    // Retrieve follower in a separate thread.
    auto follower_future = std::async(std::launch::async, [&] {
      return rpc_retrieve_standard_account(request_metadata, _return.follower_id);
    });

    // Retrieve followee in a separate thread.
    auto followee_future = std::async(std::launch::async, [&] {
      return rpc_retrieve_standard_account(request_metadata, _return.followee_id);
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
        uniquepair = rpc_get(request_metadata, follow_id);
      }
      catch (TUniquepairNotFoundException e) {
        throw TFollowNotFoundException();
      }

      // Check if requester is authorized.
      if (request_metadata.requester_id != uniquepair.first_elem)
        throw TFollowNotAuthorizedException();
    }

    // Remove unique pair.
    try {
      rpc_remove(request_metadata, follow_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TFollowNotFoundException();
    }
  }

  void list_follows(std::vector<TFollow>& _return,
      const TRequestMetadata& request_metadata, const TFollowQuery& query,
      const int32_t limit, const int32_t offset) {
    // Build query struct.
    TUniquepairQuery uniquepair_query;
    uniquepair_query.__set_domain("follow");
    if (query.__isset.follower_id)
      uniquepair_query.__set_first_elem(query.follower_id);
    if (query.__isset.followee_id)
      uniquepair_query.__set_second_elem(query.followee_id);

    // Fetch unique pairs.
    std::vector<TUniquepair> uniquepairs = rpc_fetch(request_metadata,
        uniquepair_query, limit, offset);

    // Retrieve followers in separate threads.
    std::vector<std::future<TAccount>> follower_futures;
    for (auto it : uniquepairs) {
      follower_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_retrieve_standard_account(request_metadata, it.first_elem);
      }));
    }

    // Retrieve followees in separate threads.
    std::vector<std::future<TAccount>> followee_futures;
    for (auto it : uniquepairs) {
      followee_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_retrieve_standard_account(request_metadata, it.second_elem);
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
    bool follow_exists;
    try {
      rpc_find(request_metadata, "follow", follower_id, followee_id);
      follow_exists = true;
    }
    catch (TUniquepairNotFoundException e) {
      follow_exists = false;
    }
    return follow_exists;
  }

  int32_t count_followers(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("follow");
    query.__set_second_elem(account_id);

    // Count unique pairs.
    return rpc_count(request_metadata, query);
  }

  int32_t count_followees(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("follow");
    query.__set_first_elem(account_id);

    // Count unique pairs.
    return rpc_count(request_metadata, query);
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
      ("backend_filepath", "", cxxopts::value<std::string>()->default_value(
          "/etc/opt/BuzzBlog/backend.yml"))
      ("microservice_connection_pool_size", "", cxxopts::value<int>()->default_value(
          "0"));

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();
  int acceptBacklog = result["accept_backlog"].as<int>();
  std::string backend_filepath = result["backend_filepath"].as<std::string>();
  int microservice_connection_pool_size = result["microservice_connection_pool_size"].as<int>();

  // Create server.
  auto socket = std::make_shared<TServerSocket>(host, port);
  if (acceptBacklog > 0)
    socket->setAcceptBacklog(acceptBacklog);
  TThreadedServer server(
      std::make_shared<TFollowServiceProcessor>(
          std::make_shared<TFollowServiceHandler>(backend_filepath,
              microservice_connection_pool_size)),
      socket,
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  if (threads > 0)
    server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
