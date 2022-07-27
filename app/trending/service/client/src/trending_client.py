# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TTrendingService
from buzzblog.base_client import BaseClient


class Client(BaseClient):
  def __init__(self, ip_address, port, timeout=30000, logger=None, connection_pool=None):
    super().__init__(TTrendingService.Client, ip_address, port, timeout,
        logger, connection_pool)

  def process_post(self, request_metadata, text):
    return self.rpc_wrapper(request_metadata.id, "trending:process_post")(
        self._tclient.process_post, request_metadata=request_metadata,
        text=text)

  def fetch_trending_hashtags(self, request_metadata, limit):
    return self.rpc_wrapper(request_metadata.id, "trending:fetch_trending_hashtags")(
        self._tclient.fetch_trending_hashtags, request_metadata=request_metadata,
        limit=limit)
