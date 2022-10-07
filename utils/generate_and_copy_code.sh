#!/bin/bash

# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

# This script generates Thrift client/server stubs and copy client libraries.

# Define constants.
SERVICES="account follow like post uniquepair trending wordfilter"

# Change to the parent directory.
cd "$(dirname "$(dirname "$(readlink -fm "$0")")")"

# Process command-line arguments.
set -u
while [[ $# > 1 ]]; do
  case $1 in
    * )
      echo "Invalid argument: $1"
      exit 1
  esac
  shift
  shift
done

# Remove old files and generate Thrift code.
rm -rf app/apigateway/server/site-packages
mkdir -p app/apigateway/server/site-packages/buzzblog
thrift -r --gen py -out app/apigateway/server/site-packages/buzzblog app/common/thrift/buzzblog.thrift
rm -rf app/apigateway/tests/site-packages
mkdir -p app/apigateway/tests/site-packages/buzzblog
thrift -r --gen py -out app/apigateway/tests/site-packages/buzzblog app/common/thrift/buzzblog.thrift
for service in $SERVICES
do
  # C++
  rm -rf app/$service/service/server/include
  mkdir -p app/$service/service/server/include/buzzblog/gen
  thrift -r --gen cpp -out app/$service/service/server/include/buzzblog/gen app/common/thrift/buzzblog.thrift
  # Python
  rm -rf app/$service/service/tests/site-packages
  mkdir -p app/$service/service/tests/site-packages/buzzblog
  thrift -r --gen py -out app/$service/service/tests/site-packages/buzzblog app/common/thrift/buzzblog.thrift
done

# Copy base server classes and utilities.
cp app/common/site-packages/base_client.py app/apigateway/server/site-packages/buzzblog
cp app/common/site-packages/base_client.py app/apigateway/tests/site-packages/buzzblog
cp app/common/site-packages/microservice_connection_pool.py app/apigateway/server/site-packages/buzzblog
cp app/common/site-packages/microservice_connection_pool.py app/apigateway/tests/site-packages/buzzblog
cp app/common/site-packages/microservice_connected_server.py app/apigateway/server/site-packages/buzzblog
cp app/common/site-packages/microservice_connected_server.py app/apigateway/tests/site-packages/buzzblog
cp app/common/site-packages/utils.py app/apigateway/server/site-packages/buzzblog
cp app/common/site-packages/utils.py app/apigateway/tests/site-packages/buzzblog
for service in $SERVICES
do
  cp app/common/include/utils.h app/$service/service/server/include/buzzblog
  cp app/common/include/base_server.h app/$service/service/server/include/buzzblog
  cp app/common/include/microservice_connected_server.h app/$service/service/server/include/buzzblog
  cp app/common/include/postgres_connected_server.h app/$service/service/server/include/buzzblog
  cp app/common/include/redis_connected_server.h app/$service/service/server/include/buzzblog
  cp app/common/include/microservice_connection_pool.h app/$service/service/server/include/buzzblog
  cp app/common/include/postgres_connection_pool.h app/$service/service/server/include/buzzblog
  cp app/common/include/base_client.h app/$service/service/server/include/buzzblog
  cp app/common/site-packages/base_client.py app/$service/service/tests/site-packages/buzzblog
done

# Copy service client libraries.
for service in $SERVICES
do
  for service_to_copy in $SERVICES
  do
    cp app/$service_to_copy/service/client/src/*.h app/$service/service/server/include/buzzblog
    cp app/$service_to_copy/service/client/src/*.py app/$service/service/tests/site-packages/buzzblog
  done
  cp app/$service/service/client/src/*.py app/apigateway/server/site-packages/buzzblog
  cp app/$service/service/client/src/*.py app/apigateway/tests/site-packages/buzzblog
done
