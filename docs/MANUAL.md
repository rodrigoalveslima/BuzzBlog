# User Manual
BuzzBlog is composed of an API Gateway (Python Flask application running on a
Gunicorn server), microservices (C++ Thrift applications running on
multithreaded servers), and data storage systems (Postgres database servers and
a Redis data structure server). This running example shows how to configure,
start, and test these components in your local machine.

## Configuration
### `conf/backend.yml`
In `conf/backend.yml`, set the hostnames and ports of microservices and
data storage systems. API Gateway and microservices use this file to discover
what network addresses they should connect to.
```
account:
  service:
    - "172.17.0.1:9090"
  database: "172.17.0.1:5433"
follow:
  service:
    - "172.17.0.1:9091"
like:
  service:
    - "172.17.0.1:9092"
post:
  service:
    - "172.17.0.1:9093"
  database: "172.17.0.1:5434"
uniquepair:
  service:
    - "172.17.0.1:9094"
  database: "172.17.0.1:5435"
trending:
  service:
    - "172.17.0.1:9095"
  redis: "172.17.0.1:6379"
wordfilter:
  service:
    - "172.17.0.1:9096"
```

### `conf/redis.conf`
In `conf/redis.conf`, set the Redis server configuration parameters.

## Deployment
### API Gateway
1. Generate Thrift client/server stubs and copy client libraries.
```
sudo ./utils/generate_and_copy_code.sh
```
2. Build the Docker image.
```
cd app/apigateway/server
sudo docker build -t apigateway:latest .
```
3. Start a Docker container running the newly built image (environment
variables are documented in the Dockerfile).
```
sudo docker run \
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
```

### Account Service
1. Create a Docker volume named `pg_account`.
```
sudo docker volume create pg_account
```
2. Start a Docker container based on the official PostgreSQL image.
```
sudo docker run \
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
```
3. Create tables and indexes.
```
psql -U postgres -d account -h localhost -p 5433 -f app/account/database/account_schema.sql
```
4. Generate Thrift client/server stubs and copy client libraries.
```
sudo ./utils/generate_and_copy_code.sh
```
5. Build the Docker image.
```
cd app/account/service/server
sudo docker build -t account:latest .
```
6. Start a Docker container running the newly built image (environment
variables are documented in the Dockerfile).
```
sudo docker run \
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
```

### Follow Service
1. Generate Thrift client/server stubs and copy client libraries.
```
sudo ./utils/generate_and_copy_code.sh
```
2. Build the Docker image.
```
cd app/follow/service/server
sudo docker build -t follow:latest .
```
3. Start a Docker container running the newly built image (environment
variables are documented in the Dockerfile).
```
sudo docker run \
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
```

### Like Service
1. Generate Thrift client/server stubs and copy client libraries.
```
sudo ./utils/generate_and_copy_code.sh
```
2. Build the Docker image.
```
cd app/like/service/server
sudo docker build -t like:latest .
```
3. Start a Docker container running the newly built image (environment
variables are documented in the Dockerfile).
```
sudo docker run \
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
```

### Post Service
1. Create a Docker volume named `pg_post`.
```
sudo docker volume create pg_post
```
2. Start a Docker container based on the official PostgreSQL image.
```
sudo docker run \
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
```
3. Create tables and indexes.
```
psql -U postgres -d post -h localhost -p 5434 -f app/post/database/post_schema.sql
```
4. Generate Thrift client/server stubs and copy client libraries.
```
sudo ./utils/generate_and_copy_code.sh
```
5. Build the Docker image.
```
cd app/post/service/server
sudo docker build -t post:latest .
```
6. Start a Docker container running the newly built image (environment
variables are documented in the Dockerfile).
```
sudo docker run \
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
```

### Uniquepair Service
1. Create a Docker volume named `pg_uniquepair`.
```
sudo docker volume create pg_uniquepair
```
2. Start a Docker container based on the official PostgreSQL image.
```
sudo docker run \
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
```
3. Create tables and indexes.
```
psql -U postgres -d uniquepair -h localhost -p 5435 -f app/uniquepair/database/uniquepair_schema.sql
```
4. Generate Thrift client/server stubs and copy client libraries.
```
sudo ./utils/generate_and_copy_code.sh
```
5. Build the Docker image.
```
cd app/uniquepair/service/server
sudo docker build -t uniquepair:latest .
```
6. Start a Docker container running the newly built image (environment
variables are documented in the Dockerfile).
```
sudo docker run \
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
```

### Trending Service
1. Create a Docker volume named `redis_trending`.
```
sudo docker volume create redis_trending
```
2. Start a Docker container based on the official Redis image.
```
sudo docker run \
    --name trending_redis \
    --publish 6379:6379 \
    --volume redis_trending:/data \
    --volume $(pwd)/conf/redis.conf:/usr/local/etc/redis/redis.conf \
    --detach \
    redis:6.2 \
    redis-server /usr/local/etc/redis/redis.conf
```
3. Generate Thrift client/server stubs and copy client libraries.
```
sudo ./utils/generate_and_copy_code.sh
```
4. Build the Docker image.
```
cd app/trending/service/server
sudo docker build -t trending:latest .
```
5. Start a Docker container running the newly built image (environment
variables are documented in the Dockerfile).
```
sudo docker run \
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
```

### Wordfilter Service
1. Generate Thrift client/server stubs and copy client libraries.
```
sudo ./utils/generate_and_copy_code.sh
```
2. Build the Docker image.
```
cd app/wordfilter/service/server
sudo docker build -t wordfilter:latest .
```
3. Start a Docker container running the newly built image (environment
variables are documented in the Dockerfile).
```
sudo docker run \
    --name wordfilter_service \
    --publish 9096:9096 \
    --env port=9096 \
    --env threads=1024 \
    --env accept_backlog=1024 \
    --env n_invalid_words=128 \
    --env logging=1 \
    --detach \
    wordfilter:latest
```

## Unit Testing
```
for service in account follow like post uniquepair trending wordfilter
do
  export PYTHONPATH=app/$service/service/tests/site-packages/
  python3 app/$service/service/tests/test_$service.py
done
python3 app/apigateway/tests/test_api.py
```
