#!/bin/bash

# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

# This script stop running containers and remove containers, images, and
# volumes.

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

# Stop all containers.
docker container stop $(docker container ls -aq)

# Remove all containers.
docker container rm $(docker container ls -aq)

# Remove all images.
docker rmi -f $(docker images -a -q)

# Remove all stopped containers, all networks and volumes not used by at least
# one container, all dangling and unused images, and all build cache.
docker system prune -af --volumes
