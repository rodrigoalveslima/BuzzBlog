// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef MICROSERVICE_CONNECTED_SERVER__H
#define MICROSERVICE_CONNECTED_SERVER__H

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <yaml-cpp/yaml.h>

#include <buzzblog/base_server.h>
#include <buzzblog/microservice_connection_pool.h>
#include <buzzblog/account_client.h>
#include <buzzblog/follow_client.h>
#include <buzzblog/like_client.h>
#include <buzzblog/post_client.h>
#include <buzzblog/uniquepair_client.h>
#include <buzzblog/trending_client.h>
#include <buzzblog/wordfilter_client.h>

class MicroserviceConnectedServer :
    public BaseServer {
 protected:
  MicroserviceConnectedServer(const std::string& backend_filepath,
      const int microservice_connection_pool_size) {
    // Initialize logger.
    auto rpc_logger = spdlog::basic_logger_mt("rpc_logger", "/tmp/calls.log");
    rpc_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");

    // Parse backend configuration.
    stdout_log("Initializing MicroserviceConnectedServer");
    auto backend_conf = YAML::LoadFile(backend_filepath);

    // Process backend configuration.
    std::map<std::string, std::vector<std::pair<std::string, int>>> service;
    for (const auto& it : backend_conf) {
      auto service_name = it.first.as<std::string>();
      auto service_conf = it.second;
      // Process service server configuration.
      if (service_conf["service"]) {
        for (const auto& jt : service_conf["service"]) {
          auto server_address = jt.as<std::string>();
          auto server_host = server_address.substr(0, server_address.find(":"));
          auto server_port = std::stoi(
              server_address.substr(server_address.find(":") + 1));
          service[service_name].push_back(
              std::make_pair(server_host, server_port));
          stdout_log("Added " + service_name + " service on: " + server_address);
        }
      }
    }

    // Initialize connection pools.
    _account_cp = std::make_shared<MicroserviceConnectionPool<account_service::Client>>(service["account"],
        microservice_connection_pool_size, 30000, rpc_logger);
    _follow_cp = std::make_shared<MicroserviceConnectionPool<follow_service::Client>>(service["follow"],
        microservice_connection_pool_size, 30000, rpc_logger);
    _like_cp = std::make_shared<MicroserviceConnectionPool<like_service::Client>>(service["like"],
        microservice_connection_pool_size, 30000, rpc_logger);
    _post_cp = std::make_shared<MicroserviceConnectionPool<post_service::Client>>(service["post"],
        microservice_connection_pool_size, 30000, rpc_logger);
    _uniquepair_cp = std::make_shared<MicroserviceConnectionPool<uniquepair_service::Client>>(service["uniquepair"],
        microservice_connection_pool_size, 30000, rpc_logger);
    _trending_cp = std::make_shared<MicroserviceConnectionPool<trending_service::Client>>(service["trending"],
        microservice_connection_pool_size, 30000, rpc_logger);
    _wordfilter_cp = std::make_shared<MicroserviceConnectionPool<wordfilter_service::Client>>(service["wordfilter"],
        microservice_connection_pool_size, 30000, rpc_logger);
  }

  // Account RPCs
  TAccount rpc_authenticate_user(const TRequestMetadata& request_metadata,
      const std::string& username, const std::string& password) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = account_client->authenticate_user(request_metadata, username,
          password);
    }
    catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  TAccount rpc_create_account(const TRequestMetadata& request_metadata,
      const std::string& username, const std::string& password,
      const std::string& first_name, const std::string& last_name) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = account_client->create_account(request_metadata, username, password,
          first_name, last_name);
    }
    catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  TAccount rpc_retrieve_standard_account(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = account_client->retrieve_standard_account(request_metadata,
          account_id);
    }
    catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  TAccount rpc_retrieve_expanded_account(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = account_client->retrieve_expanded_account(request_metadata,
          account_id);
    }
    catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  TAccount rpc_update_account(const TRequestMetadata& request_metadata,
      const int32_t account_id, const std::string& password,
      const std::string& first_name, const std::string& last_name) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = account_client->update_account(request_metadata, account_id, password,
          first_name, last_name);
    }
    catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  void rpc_delete_account(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    auto account_client = _account_cp->get_client();
    try {
      account_client->delete_account(request_metadata, account_id);
    }
    catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
  }

  // Follow RPCs
  TFollow rpc_follow_account(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    TFollow res;
    auto follow_client = _follow_cp->get_client();
    try {
      res = follow_client->follow_account(request_metadata, account_id);
    }
    catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
    return res;
  }

  TFollow rpc_retrieve_standard_follow(const TRequestMetadata& request_metadata,
      const int32_t follow_id) {
    TFollow res;
    auto follow_client = _follow_cp->get_client();
    try {
      res = follow_client->retrieve_standard_follow(request_metadata, follow_id);
    }
    catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
    return res;
  }

  TFollow rpc_retrieve_expanded_follow(const TRequestMetadata& request_metadata,
      const int32_t follow_id) {
    TFollow res;
    auto follow_client = _follow_cp->get_client();
    try {
      res = follow_client->retrieve_expanded_follow(request_metadata, follow_id);
    }
    catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
    return res;
  }

  void rpc_delete_follow(const TRequestMetadata& request_metadata,
      const int32_t follow_id) {
    auto follow_client = _follow_cp->get_client();
    try {
      follow_client->delete_follow(request_metadata, follow_id);
    }
    catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
  }

  std::vector<TFollow> rpc_list_follows(const TRequestMetadata& request_metadata,
      const TFollowQuery& query, const int32_t limit, const int32_t offset) {
    std::vector<TFollow> res;
    auto follow_client = _follow_cp->get_client();
    try {
      res = follow_client->list_follows(request_metadata, query, limit, offset);
    }
    catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
    return res;
  }

  bool rpc_check_follow(const TRequestMetadata& request_metadata,
      const int32_t follower_id, const int32_t followee_id) {
    bool res;
    auto follow_client = _follow_cp->get_client();
    try {
      res = follow_client->check_follow(request_metadata, follower_id, followee_id);
    }
    catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
    return res;
  }

  int32_t rpc_count_followers(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    int32_t res;
    auto follow_client = _follow_cp->get_client();
    try {
      res = follow_client->count_followers(request_metadata, account_id);
    }
    catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
    return res;
  }

  int32_t rpc_count_followees(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    int32_t res;
    auto follow_client = _follow_cp->get_client();
    try {
      res = follow_client->count_followees(request_metadata, account_id);
    }
    catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
    return res;
  }

  // Like RPCs
  TLike rpc_like_post(const TRequestMetadata& request_metadata,
      const int32_t post_id) {
    TLike res;
    auto like_client = _like_cp->get_client();
    try {
      res = like_client->like_post(request_metadata, post_id);
    }
    catch (...) {
      _like_cp->release_client(like_client);
      throw;
    }
    _like_cp->release_client(like_client);
    return res;
  }

  TLike rpc_retrieve_standard_like(const TRequestMetadata& request_metadata,
      const int32_t like_id) {
    TLike res;
    auto like_client = _like_cp->get_client();
    try {
      res = like_client->retrieve_standard_like(request_metadata, like_id);
    }
    catch (...) {
      _like_cp->release_client(like_client);
      throw;
    }
    _like_cp->release_client(like_client);
    return res;
  }

  TLike rpc_retrieve_expanded_like(const TRequestMetadata& request_metadata,
      const int32_t like_id) {
    TLike res;
    auto like_client = _like_cp->get_client();
    try {
      res = like_client->retrieve_expanded_like(request_metadata, like_id);
    }
    catch (...) {
      _like_cp->release_client(like_client);
      throw;
    }
    _like_cp->release_client(like_client);
    return res;
  }

  void rpc_delete_like(const TRequestMetadata& request_metadata,
      const int32_t like_id) {
    auto like_client = _like_cp->get_client();
    try {
      like_client->delete_like(request_metadata, like_id);
    }
    catch (...) {
      _like_cp->release_client(like_client);
      throw;
    }
    _like_cp->release_client(like_client);
  }

  std::vector<TLike> rpc_list_likes(const TRequestMetadata& request_metadata,
      const TLikeQuery& query, const int32_t limit, const int32_t offset) {
    std::vector<TLike> res;
    auto like_client = _like_cp->get_client();
    try {
      res = like_client->list_likes(request_metadata, query, limit, offset);
    }
    catch (...) {
      _like_cp->release_client(like_client);
      throw;
    }
    _like_cp->release_client(like_client);
    return res;
  }

  int32_t rpc_count_likes_by_account(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    int32_t res;
    auto like_client = _like_cp->get_client();
    try {
      res = like_client->count_likes_by_account(request_metadata, account_id);
    }
    catch (...) {
      _like_cp->release_client(like_client);
      throw;
    }
    _like_cp->release_client(like_client);
    return res;
  }

  int32_t rpc_count_likes_of_post(const TRequestMetadata& request_metadata,
      const int32_t post_id) {
    int32_t res;
    auto like_client = _like_cp->get_client();
    try {
      res = like_client->count_likes_of_post(request_metadata, post_id);
    }
    catch (...) {
      _like_cp->release_client(like_client);
      throw;
    }
    _like_cp->release_client(like_client);
    return res;
  }

  // Post RPCs
  TPost rpc_create_post(const TRequestMetadata& request_metadata,
      const std::string& text) {
    TPost res;
    auto post_client = _post_cp->get_client();
    try {
      res = post_client->create_post(request_metadata, text);
    }
    catch (...) {
      _post_cp->release_client(post_client);
      throw;
    }
    _post_cp->release_client(post_client);
    return res;
  }

  TPost rpc_retrieve_standard_post(const TRequestMetadata& request_metadata,
      const int32_t post_id) {
    TPost res;
    auto post_client = _post_cp->get_client();
    try {
      res = post_client->retrieve_standard_post(request_metadata, post_id);
    }
    catch (...) {
      _post_cp->release_client(post_client);
      throw;
    }
    _post_cp->release_client(post_client);
    return res;
  }

  TPost rpc_retrieve_expanded_post(const TRequestMetadata& request_metadata,
      const int32_t post_id) {
    TPost res;
    auto post_client = _post_cp->get_client();
    try {
      res = post_client->retrieve_expanded_post(request_metadata, post_id);
    }
    catch (...) {
      _post_cp->release_client(post_client);
      throw;
    }
    _post_cp->release_client(post_client);
    return res;
  }

  void rpc_delete_post(const TRequestMetadata& request_metadata,
      const int32_t post_id) {
    auto post_client = _post_cp->get_client();
    try {
      post_client->delete_post(request_metadata, post_id);
    }
    catch (...) {
      _post_cp->release_client(post_client);
      throw;
    }
    _post_cp->release_client(post_client);
  }

  std::vector<TPost> rpc_list_posts(const TRequestMetadata& request_metadata,
      const TPostQuery& query, const int32_t limit, const int32_t offset) {
    std::vector<TPost> res;
    auto post_client = _post_cp->get_client();
    try {
      res = post_client->list_posts(request_metadata, query, limit, offset);
    }
    catch (...) {
      _post_cp->release_client(post_client);
      throw;
    }
    _post_cp->release_client(post_client);
    return res;
  }

  int32_t rpc_count_posts_by_author(const TRequestMetadata& request_metadata,
      const int32_t author_id) {
    int32_t res;
    auto post_client = _post_cp->get_client();
    try {
      res = post_client->count_posts_by_author(request_metadata, author_id);
    }
    catch (...) {
      _post_cp->release_client(post_client);
      throw;
    }
    _post_cp->release_client(post_client);
    return res;
  }

  // Uniquepair RPCs
  TUniquepair rpc_get(const TRequestMetadata& request_metadata,
      const int32_t uniquepair_id) {
    TUniquepair res;
    auto uniquepair_client = _uniquepair_cp->get_client();
    try {
      res = uniquepair_client->get(request_metadata, uniquepair_id);
    }
    catch (...) {
      _uniquepair_cp->release_client(uniquepair_client);
      throw;
    }
    _uniquepair_cp->release_client(uniquepair_client);
    return res;
  }

  TUniquepair rpc_add(const TRequestMetadata& request_metadata,
      const std::string& domain, const int32_t first_elem,
      const int32_t second_elem) {
    TUniquepair res;
    auto uniquepair_client = _uniquepair_cp->get_client();
    try {
      res = uniquepair_client->add(request_metadata, domain, first_elem, second_elem);
    }
    catch (...) {
      _uniquepair_cp->release_client(uniquepair_client);
      throw;
    }
    _uniquepair_cp->release_client(uniquepair_client);
    return res;
  }

  void rpc_remove(const TRequestMetadata& request_metadata,
      const int32_t uniquepair_id) {
    auto uniquepair_client = _uniquepair_cp->get_client();
    try {
      uniquepair_client->remove(request_metadata, uniquepair_id);
    }
    catch (...) {
      _uniquepair_cp->release_client(uniquepair_client);
      throw;
    }
    _uniquepair_cp->release_client(uniquepair_client);
  }

  TUniquepair rpc_find(const TRequestMetadata& request_metadata,
      const std::string& domain, const int32_t first_elem,
      const int32_t second_elem) {
    TUniquepair res;
    auto uniquepair_client = _uniquepair_cp->get_client();
    try {
      res = uniquepair_client->find(request_metadata, domain, first_elem,
          second_elem);
    }
    catch (...) {
      _uniquepair_cp->release_client(uniquepair_client);
      throw;
    }
    _uniquepair_cp->release_client(uniquepair_client);
    return res;
  }

  std::vector<TUniquepair> rpc_fetch(const TRequestMetadata& request_metadata,
      const TUniquepairQuery& query, const int32_t limit,
      const int32_t offset) {
    std::vector<TUniquepair> res;
    auto uniquepair_client = _uniquepair_cp->get_client();
    try {
      res = uniquepair_client->fetch(request_metadata, query, limit, offset);
    }
    catch (...) {
      _uniquepair_cp->release_client(uniquepair_client);
      throw;
    }
    _uniquepair_cp->release_client(uniquepair_client);
    return res;
  }

  int32_t rpc_count(const TRequestMetadata& request_metadata,
      const TUniquepairQuery& query) {
    int32_t res;
    auto uniquepair_client = _uniquepair_cp->get_client();
    try {
      res = uniquepair_client->count(request_metadata, query);
    }
    catch (...) {
      _uniquepair_cp->release_client(uniquepair_client);
      throw;
    }
    _uniquepair_cp->release_client(uniquepair_client);
    return res;
  }

  // Trending RPCs
  void rpc_process_post(const TRequestMetadata& request_metadata,
      const std::string& text) {
    auto trending_client = _trending_cp->get_client();
    try {
      trending_client->process_post(request_metadata, text);
    }
    catch (...) {
      _trending_cp->release_client(trending_client);
      throw;
    }
    _trending_cp->release_client(trending_client);
  }

  std::vector<std::string> rpc_fetch_trending_hashtags(const TRequestMetadata& request_metadata,
      const int32_t limit) {
    std::vector<std::string> res;
    auto trending_client = _trending_cp->get_client();
    try {
      res = trending_client->fetch_trending_hashtags(request_metadata, limit);
    }
    catch (...) {
      _trending_cp->release_client(trending_client);
      throw;
    }
    _trending_cp->release_client(trending_client);
    return res;
  }

  // Wordfilter RPCs
  bool rpc_is_valid_word(const TRequestMetadata& request_metadata,
      const std::string& word) {
    bool res;
    auto wordfilter_client = _wordfilter_cp->get_client();
    try {
      res = wordfilter_client->is_valid_word(request_metadata, word);
    }
    catch (...) {
      _wordfilter_cp->release_client(wordfilter_client);
      throw;
    }
    _wordfilter_cp->release_client(wordfilter_client);
    return res;
  }

 private:
  // Connection pools.
  std::shared_ptr<MicroserviceConnectionPool<account_service::Client>> _account_cp;
  std::shared_ptr<MicroserviceConnectionPool<follow_service::Client>> _follow_cp;
  std::shared_ptr<MicroserviceConnectionPool<like_service::Client>> _like_cp;
  std::shared_ptr<MicroserviceConnectionPool<post_service::Client>> _post_cp;
  std::shared_ptr<MicroserviceConnectionPool<uniquepair_service::Client>> _uniquepair_cp;
  std::shared_ptr<MicroserviceConnectionPool<trending_service::Client>> _trending_cp;
  std::shared_ptr<MicroserviceConnectionPool<wordfilter_service::Client>> _wordfilter_cp;
};

#endif
