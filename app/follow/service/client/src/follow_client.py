# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TFollowService
from buzzblog.base_client import BaseClient


class Client(BaseClient):

  def __init__(self, ip_address, port, timeout=30000, connection_pool=None):
    super().__init__(TFollowService.Client, ip_address, port, timeout,
                     connection_pool)

  def follow_account(self, request_metadata, account_id):
    return self._tclient.follow_account(request_metadata=request_metadata,
                                        account_id=account_id)

  def retrieve_standard_follow(self, request_metadata, follow_id):
    return self._tclient.retrieve_standard_follow(
        request_metadata=request_metadata, follow_id=follow_id)

  def retrieve_expanded_follow(self, request_metadata, follow_id):
    return self._tclient.retrieve_expanded_follow(
        request_metadata=request_metadata, follow_id=follow_id)

  def delete_follow(self, request_metadata, follow_id):
    return self._tclient.delete_follow(request_metadata=request_metadata,
                                       follow_id=follow_id)

  def list_follows(self, request_metadata, query, limit, offset):
    return self._tclient.list_follows(request_metadata=request_metadata,
                                      query=query,
                                      limit=limit,
                                      offset=offset)

  def check_follow(self, request_metadata, follower_id, followee_id):
    return self._tclient.check_follow(request_metadata=request_metadata,
                                      follower_id=follower_id,
                                      followee_id=followee_id)

  def count_followers(self, request_metadata, account_id):
    return self._tclient.count_followers(request_metadata=request_metadata,
                                         account_id=account_id)

  def count_followees(self, request_metadata, account_id):
    return self._tclient.count_followees(request_metadata=request_metadata,
                                         account_id=account_id)
