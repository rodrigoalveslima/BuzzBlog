// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef UTILS__H
#define UTILS__H

#include <spdlog/sinks/basic_file_sink.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

template <typename U>
U RPC_WRAPPER(std::function<U()> rpc, std::shared_ptr<spdlog::logger> logger,
              const std::string& logline) {
  auto start_time = std::chrono::steady_clock::now();
  U ret = rpc();
  std::chrono::duration<double> latency =
      std::chrono::steady_clock::now() - start_time;
  if (logger)
    logger->info((logline + std::string(" lat={}")).c_str(), latency.count());
  return ret;
}

void VOID_RPC_WRAPPER(std::function<void()> rpc,
                      std::shared_ptr<spdlog::logger> logger,
                      const std::string& logline) {
  auto start_time = std::chrono::steady_clock::now();
  rpc();
  std::chrono::duration<double> latency =
      std::chrono::steady_clock::now() - start_time;
  if (logger)
    logger->info((logline + std::string(" lat={}")).c_str(), latency.count());
}

#endif
