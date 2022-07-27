// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <future>
#include <string>

#include <cxxopts.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TLikeService.h>
#include <buzzblog/microservice_connected_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;


class TLikeServiceHandler :
    public MicroserviceConnectedServer,
    public TLikeServiceIf {
public:
  TLikeServiceHandler(const std::string& backend_filepath,
      const int microservice_connection_pool_size)
  : MicroserviceConnectedServer(backend_filepath, microservice_connection_pool_size) {
  }

  void like_post(TLike& _return, const TRequestMetadata& request_metadata,
      const int32_t post_id) {
    // Add unique pair (account, post).
    TUniquepair uniquepair;
    try {
      uniquepair = rpc_add(request_metadata, "like", request_metadata.requester_id,
          post_id);
    }
    catch (TUniquepairAlreadyExistsException e) {
      throw TLikeAlreadyExistsException();
    }

    // Build like (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.account_id = request_metadata.requester_id;
    _return.post_id = post_id;
  }

  void retrieve_standard_like(TLike& _return,
      const TRequestMetadata& request_metadata, const int32_t like_id) {
    // Get unique pair.
    TUniquepair uniquepair;
    try {
      uniquepair = rpc_get(request_metadata, like_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TLikeNotFoundException();
    }

    // Build like (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.account_id = uniquepair.first_elem;
    _return.post_id = uniquepair.second_elem;
  }

  void retrieve_expanded_like(TLike& _return,
      const TRequestMetadata& request_metadata, const int32_t like_id) {
    // Retrieve standard like.
    retrieve_standard_like(_return, request_metadata, like_id);

    // Retrieve account in a separate thread.
    auto account_future = std::async(std::launch::async, [&] {
      return rpc_retrieve_standard_account(request_metadata, _return.account_id);
    });

    // Retrieve post in a separate thread.
    auto post_future = std::async(std::launch::async, [&] {
      return rpc_retrieve_expanded_post(request_metadata, _return.post_id);
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
        uniquepair = rpc_get(request_metadata, like_id);
      }
      catch (TUniquepairNotFoundException e) {
        throw TLikeNotFoundException();
      }

      // Check if requester is authorized.
      if (request_metadata.requester_id != uniquepair.first_elem)
        throw TLikeNotAuthorizedException();
    }

    // Remove unique pair.
    try {
      rpc_remove(request_metadata, like_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TLikeNotFoundException();
    }
  }

  void list_likes(std::vector<TLike>& _return,
      const TRequestMetadata& request_metadata, const TLikeQuery& query,
      const int32_t limit, const int32_t offset) {
    // Build query struct.
    TUniquepairQuery uniquepair_query;
    uniquepair_query.__set_domain("like");
    if (query.__isset.account_id)
      uniquepair_query.__set_first_elem(query.account_id);
    if (query.__isset.post_id)
      uniquepair_query.__set_second_elem(query.post_id);

    // Fetch unique pairs.
    std::vector<TUniquepair> uniquepairs = rpc_fetch(request_metadata,
        uniquepair_query, limit, offset);

    // Retrieve accounts in separate threads.
    std::vector<std::future<TAccount>> account_futures;
    for (auto it : uniquepairs) {
      account_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_retrieve_standard_account(request_metadata, it.first_elem);
      }));
    }

    // Retrieve posts in separate threads.
    std::vector<std::future<TPost>> post_futures;
    for (auto it : uniquepairs) {
      post_futures.push_back(std::async(std::launch::async, [&] {
        return rpc_retrieve_expanded_post(request_metadata, it.second_elem);
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
    return rpc_count(request_metadata, query);
  }

  int32_t count_likes_of_post(const TRequestMetadata& request_metadata,
      const int32_t post_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("like");
    query.__set_second_elem(post_id);

    // Count unique pairs.
    return rpc_count(request_metadata, query);
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
      std::make_shared<TLikeServiceProcessor>(
          std::make_shared<TLikeServiceHandler>(backend_filepath,
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
