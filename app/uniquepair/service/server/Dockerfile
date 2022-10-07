# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

# Define base configuration.
FROM ubuntu:20.04
MAINTAINER ral@gatech.edu
WORKDIR /opt/BuzzBlog/app/uniquepair/service/server

# Max number of Thrift server worker threads.
ENV threads null
# Max size of Thrift server socket accept backlog.
ENV accept_backlog null
# Thrift server port number.
ENV port null
# Backend addresses.
ENV backend_filepath null
# Min size of Thrift server Postgres connection pools.
ENV postgres_connection_pool_min_size null
# Max size of Thrift server Postgres connection pools.
ENV postgres_connection_pool_max_size null
# Allow ephemeral connections in Postgres connection pools.
ENV postgres_connection_pool_allow_ephemeral null
# Postgres username.
ENV postgres_user null
# Postgres password.
ENV postgres_password null
# Enable/Disable logging.
ENV logging null

# Install software dependencies.
RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    apt-utils \
    automake \
    bison \
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

# Add PostgreSQL repository.
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add - \
  && echo "deb http://apt.postgresql.org/pub/repos/apt/ `lsb_release -cs`-pgdg main" | tee  /etc/apt/sources.list.d/pgdg.list \
  && apt-get update

# Install PostgreSQL client 13.
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  postgresql-client-13

# Install Thrift 0.13.
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libthrift-0.13.0=0.13.0-2build2 \
  libthrift-dev=0.13.0-2build2

# Install libpqxx 6.4.5.
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libpqxx-6.4=6.4.5-2build1 \
  libpqxx-dev=6.4.5-2build1

# Install libyaml 0.6.2.
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libyaml-cpp0.6=0.6.2-4ubuntu1 \
  libyaml-cpp-dev=0.6.2-4ubuntu1
    
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
RUN mkdir bin && g++ -o bin/uniquepair_server src/uniquepair_server.cpp \
    include/buzzblog/gen/buzzblog_types.cpp \
    include/buzzblog/gen/buzzblog_constants.cpp \
    include/buzzblog/gen/TAccountService.cpp \
    include/buzzblog/gen/TFollowService.cpp \
    include/buzzblog/gen/TLikeService.cpp \
    include/buzzblog/gen/TPostService.cpp \
    include/buzzblog/gen/TUniquepairService.cpp \
    include/buzzblog/gen/TTrendingService.cpp \
    include/buzzblog/gen/TWordfilterService.cpp \
    -std=c++2a -lthrift -lpqxx -lpq -lyaml-cpp \
    -I/opt/BuzzBlog/app/uniquepair/service/server/include \
    -I/usr/local/include

# Start the server.
CMD ["/bin/bash", "-c", "bin/uniquepair_server --host 0.0.0.0 --threads $threads --accept_backlog $accept_backlog --port $port --backend_filepath $backend_filepath --postgres_connection_pool_min_size $postgres_connection_pool_min_size --postgres_connection_pool_max_size $postgres_connection_pool_max_size --postgres_connection_pool_allow_ephemeral $postgres_connection_pool_allow_ephemeral --postgres_user $postgres_user --postgres_password $postgres_password --logging=$logging"]
