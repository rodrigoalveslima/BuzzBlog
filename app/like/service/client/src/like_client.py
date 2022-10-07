# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TLikeService
from buzzblog.base_client import BaseClient


class Client(BaseClient):

  def __init__(self, ip_address, port, timeout=30000, connection_pool=None):
    super().__init__(TLikeService.Client, ip_address, port, timeout,
                     connection_pool)

  def like_post(self, request_metadata, post_id):
    return self._tclient.like_post(request_metadata=request_metadata,
                                   post_id=post_id)

  def retrieve_standard_like(self, request_metadata, like_id):
    return self._tclient.retrieve_standard_like(
        request_metadata=request_metadata, like_id=like_id)

  def retrieve_expanded_like(self, request_metadata, like_id):
    return self._tclient.retrieve_expanded_like(
        request_metadata=request_metadata, like_id=like_id)

  def delete_like(self, request_metadata, like_id):
    return self._tclient.delete_like(request_metadata=request_metadata,
                                     like_id=like_id)

  def list_likes(self, request_metadata, query, limit, offset):
    return self._tclient.list_likes(request_metadata=request_metadata,
                                    query=query,
                                    limit=limit,
                                    offset=offset)

  def count_likes_by_account(self, request_metadata, account_id):
    return self._tclient.count_likes_by_account(
        request_metadata=request_metadata, account_id=account_id)

  def count_likes_of_post(self, request_metadata, post_id):
    return self._tclient.count_likes_of_post(request_metadata=request_metadata,
                                             post_id=post_id)
