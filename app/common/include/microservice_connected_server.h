// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef MICROSERVICE_CONNECTED_SERVER__H
#define MICROSERVICE_CONNECTED_SERVER__H

#include <buzzblog/account_client.h>
#include <buzzblog/base_server.h>
#include <buzzblog/follow_client.h>
#include <buzzblog/like_client.h>
#include <buzzblog/microservice_connection_pool.h>
#include <buzzblog/post_client.h>
#include <buzzblog/trending_client.h>
#include <buzzblog/uniquepair_client.h>
#include <buzzblog/utils.h>
#include <buzzblog/wordfilter_client.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <yaml-cpp/yaml.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class MicroserviceConnectedServer : public BaseServer {
 protected:
  MicroserviceConnectedServer(
      const std::string& local_service_name,
      const std::string& backend_filepath,
      const int microservice_connection_pool_min_size,
      const int microservice_connection_pool_max_size,
      const bool microservice_connection_pool_allow_ephemeral,
      const int logging) {
    _local_service_name = local_service_name;
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
          auto server_port =
              std::stoi(server_address.substr(server_address.find(":") + 1));
          service[service_name].push_back(
              std::make_pair(server_host, server_port));
          stdout_log("Added " + service_name +
                     " service on: " + server_address);
        }
      }
    }

    // Initialize loggers.
    std::shared_ptr<spdlog::logger> rpc_conn_logger;
    if (logging) {
      _rpc_call_logger =
          spdlog::basic_logger_mt("rpc_call_logger", "/tmp/rpc_call.log");
      _rpc_call_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
      rpc_conn_logger =
          spdlog::basic_logger_mt("rpc_conn_logger", "/tmp/rpc_conn.log");
      rpc_conn_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v");
    } else {
      _rpc_call_logger = nullptr;
      rpc_conn_logger = nullptr;
    }

    // Initialize connection pools.
    _account_cp =
        std::make_shared<MicroserviceConnectionPool<account_service::Client>>(
            local_service_name, "account", service["account"],
            microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral, 30000,
            rpc_conn_logger);
    _follow_cp =
        std::make_shared<MicroserviceConnectionPool<follow_service::Client>>(
            local_service_name, "follow", service["follow"],
            microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral, 30000,
            rpc_conn_logger);
    _like_cp =
        std::make_shared<MicroserviceConnectionPool<like_service::Client>>(
            local_service_name, "like", service["like"],
            microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral, 30000,
            rpc_conn_logger);
    _post_cp =
        std::make_shared<MicroserviceConnectionPool<post_service::Client>>(
            local_service_name, "post", service["post"],
            microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral, 30000,
            rpc_conn_logger);
    _uniquepair_cp = std::make_shared<
        MicroserviceConnectionPool<uniquepair_service::Client>>(
        local_service_name, "uniquepair", service["uniquepair"],
        microservice_connection_pool_min_size,
        microservice_connection_pool_max_size,
        microservice_connection_pool_allow_ephemeral, 30000, rpc_conn_logger);
    _trending_cp =
        std::make_shared<MicroserviceConnectionPool<trending_service::Client>>(
            local_service_name, "trending", service["trending"],
            microservice_connection_pool_min_size,
            microservice_connection_pool_max_size,
            microservice_connection_pool_allow_ephemeral, 30000,
            rpc_conn_logger);
    _wordfilter_cp = std::make_shared<
        MicroserviceConnectionPool<wordfilter_service::Client>>(
        local_service_name, "wordfilter", service["wordfilter"],
        microservice_connection_pool_min_size,
        microservice_connection_pool_max_size,
        microservice_connection_pool_allow_ephemeral, 30000, rpc_conn_logger);
  }

  // Account RPCs
  TAccount rpc_authenticate_user(const TRequestMetadata& request_metadata,
                                 const std::string& username,
                                 const std::string& password) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = RPC_WRAPPER<TAccount>(
          std::bind(&account_service::Client::authenticate_user, account_client,
                    std::ref(request_metadata), std::ref(username),
                    std::ref(password)),
          _rpc_call_logger,
          "rs=account rf=authenticate_user ls=" + _local_service_name);
    } catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  TAccount rpc_create_account(const TRequestMetadata& request_metadata,
                              const std::string& username,
                              const std::string& password,
                              const std::string& first_name,
                              const std::string& last_name) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = RPC_WRAPPER<TAccount>(
          std::bind(&account_service::Client::create_account, account_client,
                    std::ref(request_metadata), std::ref(username),
                    std::ref(password), std::ref(first_name),
                    std::ref(last_name)),
          _rpc_call_logger,
          "rs=account rf=create_account ls=" + _local_service_name);
    } catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  TAccount rpc_retrieve_standard_account(
      const TRequestMetadata& request_metadata, const int32_t account_id) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = RPC_WRAPPER<TAccount>(
          std::bind(&account_service::Client::retrieve_standard_account,
                    account_client, std::ref(request_metadata),
                    std::ref(account_id)),
          _rpc_call_logger,
          "rs=account rf=retrieve_standard_account ls=" + _local_service_name);
    } catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  TAccount rpc_retrieve_expanded_account(
      const TRequestMetadata& request_metadata, const int32_t account_id) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = RPC_WRAPPER<TAccount>(
          std::bind(&account_service::Client::retrieve_expanded_account,
                    account_client, std::ref(request_metadata),
                    std::ref(account_id)),
          _rpc_call_logger,
          "rs=account rf=retrieve_expanded_account ls=" + _local_service_name);
    } catch (...) {
      _account_cp->release_client(account_client);
      throw;
    }
    _account_cp->release_client(account_client);
    return res;
  }

  TAccount rpc_update_account(const TRequestMetadata& request_metadata,
                              const int32_t account_id,
                              const std::string& password,
                              const std::string& first_name,
                              const std::string& last_name) {
    TAccount res;
    auto account_client = _account_cp->get_client();
    try {
      res = RPC_WRAPPER<TAccount>(
          std::bind(&account_service::Client::update_account, account_client,
                    std::ref(request_metadata), std::ref(account_id),
                    std::ref(password), std::ref(first_name),
                    std::ref(last_name)),
          _rpc_call_logger,
          "rs=account rf=update_account ls=" + _local_service_name);
    } catch (...) {
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
      VOID_RPC_WRAPPER(
          std::bind(&account_service::Client::delete_account, account_client,
                    std::ref(request_metadata), std::ref(account_id)),
          _rpc_call_logger,
          "rs=account rf=delete_account ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TFollow>(
          std::bind(&follow_service::Client::follow_account, follow_client,
                    std::ref(request_metadata), std::ref(account_id)),
          _rpc_call_logger,
          "rs=follow rf=follow_account ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TFollow>(
          std::bind(&follow_service::Client::retrieve_standard_follow,
                    follow_client, std::ref(request_metadata),
                    std::ref(follow_id)),
          _rpc_call_logger,
          "rs=follow rf=retrieve_standard_follow ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TFollow>(
          std::bind(&follow_service::Client::retrieve_expanded_follow,
                    follow_client, std::ref(request_metadata),
                    std::ref(follow_id)),
          _rpc_call_logger,
          "rs=follow rf=retrieve_expanded_follow ls=" + _local_service_name);
    } catch (...) {
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
      VOID_RPC_WRAPPER(
          std::bind(&follow_service::Client::delete_follow, follow_client,
                    std::ref(request_metadata), std::ref(follow_id)),
          _rpc_call_logger,
          "rs=follow rf=delete_follow ls=" + _local_service_name);
    } catch (...) {
      _follow_cp->release_client(follow_client);
      throw;
    }
    _follow_cp->release_client(follow_client);
  }

  std::vector<TFollow> rpc_list_follows(
      const TRequestMetadata& request_metadata, const TFollowQuery& query,
      const int32_t limit, const int32_t offset) {
    std::vector<TFollow> res;
    auto follow_client = _follow_cp->get_client();
    try {
      res = RPC_WRAPPER<std::vector<TFollow>>(
          std::bind(&follow_service::Client::list_follows, follow_client,
                    std::ref(request_metadata), std::ref(query),
                    std::ref(limit), std::ref(offset)),
          _rpc_call_logger,
          "rs=follow rf=list_follows ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<bool>(
          std::bind(&follow_service::Client::check_follow, follow_client,
                    std::ref(request_metadata), std::ref(follower_id),
                    std::ref(followee_id)),
          _rpc_call_logger,
          "rs=follow rf=check_follow ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<int32_t>(
          std::bind(&follow_service::Client::count_followers, follow_client,
                    std::ref(request_metadata), std::ref(account_id)),
          _rpc_call_logger,
          "rs=follow rf=count_followers ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<int32_t>(
          std::bind(&follow_service::Client::count_followees, follow_client,
                    std::ref(request_metadata), std::ref(account_id)),
          _rpc_call_logger,
          "rs=follow rf=count_followees ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TLike>(
          std::bind(&like_service::Client::like_post, like_client,
                    std::ref(request_metadata), std::ref(post_id)),
          _rpc_call_logger, "rs=like rf=like_post ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TLike>(
          std::bind(&like_service::Client::retrieve_standard_like, like_client,
                    std::ref(request_metadata), std::ref(like_id)),
          _rpc_call_logger,
          "rs=like rf=retrieve_standard_like ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TLike>(
          std::bind(&like_service::Client::retrieve_expanded_like, like_client,
                    std::ref(request_metadata), std::ref(like_id)),
          _rpc_call_logger,
          "rs=like rf=retrieve_expanded_like ls=" + _local_service_name);
    } catch (...) {
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
      VOID_RPC_WRAPPER(
          std::bind(&like_service::Client::delete_like, like_client,
                    std::ref(request_metadata), std::ref(like_id)),
          _rpc_call_logger, "rs=like rf=delete_like ls=" + _local_service_name);
    } catch (...) {
      _like_cp->release_client(like_client);
      throw;
    }
    _like_cp->release_client(like_client);
  }

  std::vector<TLike> rpc_list_likes(const TRequestMetadata& request_metadata,
                                    const TLikeQuery& query,
                                    const int32_t limit, const int32_t offset) {
    std::vector<TLike> res;
    auto like_client = _like_cp->get_client();
    try {
      res = RPC_WRAPPER<std::vector<TLike>>(
          std::bind(&like_service::Client::list_likes, like_client,
                    std::ref(request_metadata), std::ref(query),
                    std::ref(limit), std::ref(offset)),
          _rpc_call_logger, "rs=like rf=list_likes ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<int32_t>(
          std::bind(&like_service::Client::count_likes_by_account, like_client,
                    std::ref(request_metadata), std::ref(account_id)),
          _rpc_call_logger,
          "rs=like rf=count_likes_by_account ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<int32_t>(
          std::bind(&like_service::Client::count_likes_of_post, like_client,
                    std::ref(request_metadata), std::ref(post_id)),
          _rpc_call_logger,
          "rs=like rf=count_likes_of_post ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TPost>(
          std::bind(&post_service::Client::create_post, post_client,
                    std::ref(request_metadata), std::ref(text)),
          _rpc_call_logger, "rs=post rf=create_post ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TPost>(
          std::bind(&post_service::Client::retrieve_standard_post, post_client,
                    std::ref(request_metadata), std::ref(post_id)),
          _rpc_call_logger,
          "rs=post rf=retrieve_standard_post ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TPost>(
          std::bind(&post_service::Client::retrieve_expanded_post, post_client,
                    std::ref(request_metadata), std::ref(post_id)),
          _rpc_call_logger,
          "rs=post rf=retrieve_expanded_post ls=" + _local_service_name);
    } catch (...) {
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
      VOID_RPC_WRAPPER(
          std::bind(&post_service::Client::delete_post, post_client,
                    std::ref(request_metadata), std::ref(post_id)),
          _rpc_call_logger, "rs=post rf=delete_post ls=" + _local_service_name);
    } catch (...) {
      _post_cp->release_client(post_client);
      throw;
    }
    _post_cp->release_client(post_client);
  }

  std::vector<TPost> rpc_list_posts(const TRequestMetadata& request_metadata,
                                    const TPostQuery& query,
                                    const int32_t limit, const int32_t offset) {
    std::vector<TPost> res;
    auto post_client = _post_cp->get_client();
    try {
      res = RPC_WRAPPER<std::vector<TPost>>(
          std::bind(&post_service::Client::list_posts, post_client,
                    std::ref(request_metadata), std::ref(query),
                    std::ref(limit), std::ref(offset)),
          _rpc_call_logger, "rs=post rf=list_posts ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<int32_t>(
          std::bind(&post_service::Client::count_posts_by_author, post_client,
                    std::ref(request_metadata), std::ref(author_id)),
          _rpc_call_logger,
          "rs=post rf=count_posts_by_author ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TUniquepair>(
          std::bind(&uniquepair_service::Client::get, uniquepair_client,
                    std::ref(request_metadata), std::ref(uniquepair_id)),
          _rpc_call_logger, "rs=uniquepair rf=get ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<TUniquepair>(
          std::bind(&uniquepair_service::Client::add, uniquepair_client,
                    std::ref(request_metadata), std::ref(domain),
                    std::ref(first_elem), std::ref(second_elem)),
          _rpc_call_logger, "rs=uniquepair rf=add ls=" + _local_service_name);
    } catch (...) {
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
      VOID_RPC_WRAPPER(
          std::bind(&uniquepair_service::Client::remove, uniquepair_client,
                    std::ref(request_metadata), std::ref(uniquepair_id)),
          _rpc_call_logger,
          "rs=uniquepair rf=remove ls=" + _local_service_name);
    } catch (...) {
      _uniquepair_cp->release_client(uniquepair_client);
      throw;
    }
    _uniquepair_cp->release_client(uniquepair_client);
  }

  bool rpc_find(const TRequestMetadata& request_metadata,
                const std::string& domain, const int32_t first_elem,
                const int32_t second_elem) {
    bool res;
    auto uniquepair_client = _uniquepair_cp->get_client();
    try {
      res = RPC_WRAPPER<bool>(
          std::bind(&uniquepair_service::Client::find, uniquepair_client,
                    std::ref(request_metadata), std::ref(domain),
                    std::ref(first_elem), std::ref(second_elem)),
          _rpc_call_logger, "rs=uniquepair rf=find ls=" + _local_service_name);
    } catch (...) {
      _uniquepair_cp->release_client(uniquepair_client);
      throw;
    }
    _uniquepair_cp->release_client(uniquepair_client);
    return res;
  }

  std::vector<TUniquepair> rpc_fetch(const TRequestMetadata& request_metadata,
                                     const TUniquepairQuery& query,
                                     const int32_t limit,
                                     const int32_t offset) {
    std::vector<TUniquepair> res;
    auto uniquepair_client = _uniquepair_cp->get_client();
    try {
      res = RPC_WRAPPER<std::vector<TUniquepair>>(
          std::bind(&uniquepair_service::Client::fetch, uniquepair_client,
                    std::ref(request_metadata), std::ref(query),
                    std::ref(limit), std::ref(offset)),
          _rpc_call_logger, "rs=uniquepair rf=fetch ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<int32_t>(
          std::bind(&uniquepair_service::Client::count, uniquepair_client,
                    std::ref(request_metadata), std::ref(query)),
          _rpc_call_logger, "rs=uniquepair rf=count ls=" + _local_service_name);
    } catch (...) {
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
      VOID_RPC_WRAPPER(
          std::bind(&trending_service::Client::process_post, trending_client,
                    std::ref(request_metadata), std::ref(text)),
          _rpc_call_logger,
          "rs=trending rf=process_post ls=" + _local_service_name);
    } catch (...) {
      _trending_cp->release_client(trending_client);
      throw;
    }
    _trending_cp->release_client(trending_client);
  }

  std::vector<std::string> rpc_fetch_trending_hashtags(
      const TRequestMetadata& request_metadata, const int32_t limit) {
    std::vector<std::string> res;
    auto trending_client = _trending_cp->get_client();
    try {
      res = RPC_WRAPPER<std::vector<std::string>>(
          std::bind(&trending_service::Client::fetch_trending_hashtags,
                    trending_client, std::ref(request_metadata),
                    std::ref(limit)),
          _rpc_call_logger,
          "rs=trending rf=fetch_trending_hashtags ls=" + _local_service_name);
    } catch (...) {
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
      res = RPC_WRAPPER<bool>(
          std::bind(&wordfilter_service::Client::is_valid_word,
                    wordfilter_client, std::ref(request_metadata),
                    std::ref(word)),
          _rpc_call_logger,
          "rs=wordfilter rf=is_valid_word ls=" + _local_service_name);
    } catch (...) {
      _wordfilter_cp->release_client(wordfilter_client);
      throw;
    }
    _wordfilter_cp->release_client(wordfilter_client);
    return res;
  }

 private:
  std::string _local_service_name;
  std::shared_ptr<spdlog::logger> _rpc_call_logger;
  // Connection pools.
  std::shared_ptr<MicroserviceConnectionPool<account_service::Client>>
      _account_cp;
  std::shared_ptr<MicroserviceConnectionPool<follow_service::Client>>
      _follow_cp;
  std::shared_ptr<MicroserviceConnectionPool<like_service::Client>> _like_cp;
  std::shared_ptr<MicroserviceConnectionPool<post_service::Client>> _post_cp;
  std::shared_ptr<MicroserviceConnectionPool<uniquepair_service::Client>>
      _uniquepair_cp;
  std::shared_ptr<MicroserviceConnectionPool<trending_service::Client>>
      _trending_cp;
  std::shared_ptr<MicroserviceConnectionPool<wordfilter_service::Client>>
      _wordfilter_cp;
};

#endif
