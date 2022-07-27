# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

# Define base configuration.
FROM ubuntu:20.04
MAINTAINER ral@gatech.edu
WORKDIR /opt/BuzzBlog/app/apigateway/service/server

# Max number of Gunicorn server workers.
ENV workers null
# Max number of threads per Gunicorn server worker.
ENV threads null
# Gunicorn server port number.
ENV port null
# Size of Thrift server microservice connection pools.
ENV microservice_connection_pool_size null

# Install software dependencies.
RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    python3-pip
RUN pip3 install --no-cache-dir gunicorn==20.1.0 && \
    pip3 install --no-cache-dir Werkzeug==2.0.3 && \
    pip3 install --no-cache-dir Flask-HTTPAuth==4.2.0 && \
    pip3 install --no-cache-dir PyYaml==5.3.1 && \
    pip3 install --no-cache-dir pybind11==2.9.2 && \
    pip3 install --no-cache-dir spdlog==2.0.4 && \
    pip3 install --no-cache-dir thrift==0.13.0

# Copy service client libraries.
COPY site-packages site-packages

# Copy source code.
COPY src src

# Create logger directory.
RUN mkdir /tmp/logger

# Start the server.
CMD ["/bin/bash", "-c", "PYTHONPATH=/opt/BuzzBlog/app/apigateway/service/server/site-packages gunicorn src.apigateway:app --workers=$workers --threads=$threads --bind=0.0.0.0:$port --env microservice_connection_pool_size=$microservice_connection_pool_size"]
