# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TPostService
from buzzblog.base_client import BaseClient


class Client(BaseClient):

  def __init__(self, ip_address, port, timeout=30000, connection_pool=None):
    super().__init__(TPostService.Client, ip_address, port, timeout,
                     connection_pool)

  def create_post(self, request_metadata, text):
    return self._tclient.create_post(request_metadata=request_metadata,
                                     text=text)

  def retrieve_standard_post(self, request_metadata, post_id):
    return self._tclient.retrieve_standard_post(
        request_metadata=request_metadata, post_id=post_id)

  def retrieve_expanded_post(self, request_metadata, post_id):
    return self._tclient.retrieve_expanded_post(
        request_metadata=request_metadata, post_id=post_id)

  def delete_post(self, request_metadata, post_id):
    return self._tclient.delete_post(request_metadata=request_metadata,
                                     post_id=post_id)

  def list_posts(self, request_metadata, query, limit, offset):
    return self._tclient.list_posts(request_metadata=request_metadata,
                                    query=query,
                                    limit=limit,
                                    offset=offset)

  def count_posts_by_author(self, request_metadata, author_id):
    return self._tclient.count_posts_by_author(
        request_metadata=request_metadata, author_id=author_id)
