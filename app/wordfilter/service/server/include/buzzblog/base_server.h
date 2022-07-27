// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#ifndef BASE_SERVER__H
#define BASE_SERVER__H

#include <ctime>
#include <iomanip>
#include <iostream>
#include <thread>
#include <unistd.h>

class BaseServer {
 protected:
  void stdout_log(const std::string& log) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::cout << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S]") << \
        " pid=" << getpid() << \
        " tid=" << std::this_thread::get_id() << \
        " " << log << std::endl;
  }
};

#endif
