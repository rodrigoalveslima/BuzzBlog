#!/bin/bash

# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

# This script deploys BuzzBlog in your local machine, as described in
# 'docs/MANUAL.md', and runs unit tests of all components.

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

# Clean Docker artifacts.
utils/clean_docker.sh

# Format code.
utils/format_code.sh

# Generate Thrift code and copy service client libraries.
utils/generate_and_copy_code.sh

# Deploy API Gateway (1 Flask server).
cd app/apigateway/server
docker build -t apigateway:latest .
cd ../../..
docker run \
    --name apigateway \
    --publish 8080:81 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlog/backend.yml \
    --env workers=2 \
    --env threads=512 \
    --env port=81 \
    --env microservice_connection_pool_min_size=16 \
    --env microservice_connection_pool_max_size=16 \
    --env microservice_connection_pool_allow_ephemeral=1 \
    --env logging=1 \
    --detach \
    apigateway:latest

# Deploy Account Service (1 PostgreSQL database server + 1 Thrift multithreaded
# server).
docker volume create pg_account
docker run \
    --name account_database \
    --publish 5433:5432 \
    --volume pg_account:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=account \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128
sleep 4
psql -U postgres -d account -h localhost -p 5433 -f app/account/database/account_schema.sql
cd app/account/service/server
docker build -t account:latest .
cd ../../../..
docker run \
    --name account_service \
    --publish 9090:9090 \
    --env port=9090 \
    --env threads=1024 \
    --env accept_backlog=1024 \
    --env backend_filepath=/etc/opt/BuzzBlog/backend.yml \
    --env microservice_connection_pool_min_size=8 \
    --env microservice_connection_pool_max_size=8 \
    --env microservice_connection_pool_allow_ephemeral=1 \
    --env postgres_connection_pool_min_size=16 \
    --env postgres_connection_pool_max_size=16 \
    --env postgres_connection_pool_allow_ephemeral=1 \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env logging=1 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlog/backend.yml \
    --detach \
    account:latest

# Deploy Follow Service (1 Thrift multithreaded server).
cd app/follow/service/server
docker build -t follow:latest .
cd ../../../..
docker run \
    --name follow_service \
    --publish 9091:9091 \
    --env port=9091 \
    --env threads=1024 \
    --env accept_backlog=1024 \
    --env backend_filepath=/etc/opt/BuzzBlog/backend.yml \
    --env microservice_connection_pool_min_size=8 \
    --env microservice_connection_pool_max_size=8 \
    --env microservice_connection_pool_allow_ephemeral=1 \
    --env logging=1 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlog/backend.yml \
    --detach \
    follow:latest

# Deploy Like Service (1 Thrift multithreaded server).
cd app/like/service/server
docker build -t like:latest .
cd ../../../..
docker run \
    --name like_service \
    --publish 9092:9092 \
    --env port=9092 \
    --env threads=1024 \
    --env accept_backlog=1024 \
    --env backend_filepath=/etc/opt/BuzzBlog/backend.yml \
    --env microservice_connection_pool_min_size=8 \
    --env microservice_connection_pool_max_size=8 \
    --env microservice_connection_pool_allow_ephemeral=1 \
    --env logging=1 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlog/backend.yml \
    --detach \
    like:latest

# Deploy Post Service (1 PostgreSQL database server + 1 Thrift multithreaded
# server).
docker volume create pg_post
docker run \
    --name post_database \
    --publish 5434:5432 \
    --volume pg_post:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=post \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128
sleep 4
psql -U postgres -d post -h localhost -p 5434 -f app/post/database/post_schema.sql
cd app/post/service/server
docker build -t post:latest .
cd ../../../..
docker run \
    --name post_service \
    --publish 9093:9093 \
    --env port=9093 \
    --env threads=1024 \
    --env accept_backlog=1024 \
    --env backend_filepath=/etc/opt/BuzzBlog/backend.yml \
    --env microservice_connection_pool_min_size=8 \
    --env microservice_connection_pool_max_size=8 \
    --env microservice_connection_pool_allow_ephemeral=1 \
    --env postgres_connection_pool_min_size=16 \
    --env postgres_connection_pool_max_size=16 \
    --env postgres_connection_pool_allow_ephemeral=1 \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env logging=1 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlog/backend.yml \
    --detach \
    post:latest

# Deploy Uniquepair Service (1 PostgreSQL database server + 1 Thrift
# multithreaded server).
docker volume create pg_uniquepair
docker run \
    --name uniquepair_database \
    --publish 5435:5432 \
    --volume pg_uniquepair:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=uniquepair \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128
sleep 4
psql -U postgres -d uniquepair -h localhost -p 5435 -f app/uniquepair/database/uniquepair_schema.sql
cd app/uniquepair/service/server
docker build -t uniquepair:latest .
cd ../../../..
docker run \
    --name uniquepair_service \
    --publish 9094:9094 \
    --env port=9094 \
    --env threads=1024 \
    --env accept_backlog=1024 \
    --env backend_filepath=/etc/opt/BuzzBlog/backend.yml \
    --env postgres_connection_pool_min_size=16 \
    --env postgres_connection_pool_max_size=16 \
    --env postgres_connection_pool_allow_ephemeral=1 \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env logging=1 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlog/backend.yml \
    --detach \
    uniquepair:latest

# Deploy Trending Service (1 Redis server + 1 Thrift multithreaded server).
docker volume create redis_trending
docker run \
    --name trending_redis \
    --publish 6379:6379 \
    --volume redis_trending:/data \
    --volume $(pwd)/conf:/usr/local/etc/redis \
    --detach \
    redis:6.2 \
    redis-server /usr/local/etc/redis/redis.conf
sleep 4
cd app/trending/service/server
docker build -t trending:latest .
cd ../../../..
docker run \
    --name trending_service \
    --publish 9095:9095 \
    --env port=9095 \
    --env threads=1024 \
    --env accept_backlog=1024 \
    --env backend_filepath=/etc/opt/BuzzBlog/backend.yml \
    --env microservice_connection_pool_min_size=8 \
    --env microservice_connection_pool_max_size=8 \
    --env microservice_connection_pool_allow_ephemeral=1 \
    --env redis_connection_pool_size=32 \
    --env logging=1 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlog/backend.yml \
    --detach \
    trending:latest

# Deploy Wordfilter Service (1 Thrift multithreaded server).
cd app/wordfilter/service/server
docker build -t wordfilter:latest .
cd ../../../..
docker run \
    --name wordfilter_service \
    --publish 9096:9096 \
    --env port=9096 \
    --env threads=1024 \
    --env accept_backlog=1024 \
    --env n_invalid_words=128 \
    --env logging=1 \
    --detach \
    wordfilter:latest

# Run unit tests for all services and the API Gateway.
for service in account follow like post uniquepair trending wordfilter
do
  export PYTHONPATH=app/$service/service/tests/site-packages/
  python3 app/$service/service/tests/test_$service.py
done
export PYTHONPATH=app/apigateway/tests/site-packages/
python3 app/apigateway/tests/test_api.py
