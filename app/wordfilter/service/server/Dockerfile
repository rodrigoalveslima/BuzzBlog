# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

# Define base configuration.
FROM ubuntu:20.04
MAINTAINER ral@gatech.edu
WORKDIR /opt/BuzzBlog/app/wordfilter/service/server

# Max number of Thrift server worker threads.
ENV threads null
# Max size of Thrift server socket accept backlog.
ENV accept_backlog null
# Thrift server port number.
ENV port null
# Number of invalid words.
ENV n_invalid_words null
# Enable/Disable logging.
ENV logging null

# Install software dependencies.
RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    apt-utils \
    automake \
    bison \
    cmake \
    flex \
    g++ \
    git \
    gnupg2 \
    libboost-all-dev \
    libevent-dev \
    libspdlog-dev \
    libssl-dev \
    libtool \
    lsb-core \
    make \
    pkg-config \
    wget \
    unzip

# Install Thrift 0.13.
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libthrift-0.13.0=0.13.0-2build2 \
  libthrift-dev=0.13.0-2build2

# Copy cxxopts 2.2.1.
RUN cd /tmp \
  && wget https://github.com/jarro2783/cxxopts/archive/v2.2.1.zip \
  && unzip v2.2.1.zip \
  && cp cxxopts-2.2.1/include/cxxopts.hpp /usr/local/include

# Copy service client libraries.
COPY include include

# Copy source code.
COPY src src

# Compile source code.
RUN mkdir bin && g++ -o bin/wordfilter_server src/wordfilter_server.cpp \
    include/buzzblog/gen/buzzblog_types.cpp \
    include/buzzblog/gen/buzzblog_constants.cpp \
    include/buzzblog/gen/TAccountService.cpp \
    include/buzzblog/gen/TFollowService.cpp \
    include/buzzblog/gen/TLikeService.cpp \
    include/buzzblog/gen/TPostService.cpp \
    include/buzzblog/gen/TUniquepairService.cpp \
    include/buzzblog/gen/TTrendingService.cpp \
    include/buzzblog/gen/TWordfilterService.cpp \
    -std=c++2a -lthrift \
    -I/opt/BuzzBlog/app/wordfilter/service/server/include \
    -I/usr/local/include

# Start the server.
CMD ["/bin/bash", "-c", "bin/wordfilter_server --host 0.0.0.0 --threads $threads --accept_backlog $accept_backlog --port $port --n_invalid_words $n_invalid_words --logging=$logging"]
