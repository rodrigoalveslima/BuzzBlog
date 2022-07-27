# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TFollowService
from buzzblog.base_client import BaseClient


class Client(BaseClient):
  def __init__(self, ip_address, port, timeout=30000, logger=None, connection_pool=None):
    super().__init__(TFollowService.Client, ip_address, port, timeout, logger, connection_pool)

  def follow_account(self, request_metadata, account_id):
    return self.rpc_wrapper(request_metadata.id, "follow:follow_account")(
        self._tclient.follow_account, request_metadata=request_metadata,
        account_id=account_id)

  def retrieve_standard_follow(self, request_metadata, follow_id):
    return self.rpc_wrapper(request_metadata.id, "follow:retrieve_standard_follow")(
        self._tclient.retrieve_standard_follow,
        request_metadata=request_metadata, follow_id=follow_id)

  def retrieve_expanded_follow(self, request_metadata, follow_id):
    return self.rpc_wrapper(request_metadata.id, "follow:retrieve_expanded_follow")(
        self._tclient.retrieve_expanded_follow,
        request_metadata=request_metadata, follow_id=follow_id)

  def delete_follow(self, request_metadata, follow_id):
    return self.rpc_wrapper(request_metadata.id, "follow:delete_follow")(
        self._tclient.delete_follow, request_metadata=request_metadata,
        follow_id=follow_id)

  def list_follows(self, request_metadata, query, limit, offset):
    return self.rpc_wrapper(request_metadata.id, "follow:list_follows")(
        self._tclient.list_follows, request_metadata=request_metadata,
        query=query, limit=limit, offset=offset)

  def check_follow(self, request_metadata, follower_id, followee_id):
    return self.rpc_wrapper(request_metadata.id, "follow:check_follow")(
        self._tclient.check_follow, request_metadata=request_metadata,
        follower_id=follower_id, followee_id=followee_id)

  def count_followers(self, request_metadata, account_id):
    return self.rpc_wrapper(request_metadata.id, "follow:count_followers")(
        self._tclient.count_followers, request_metadata=request_metadata,
        account_id=account_id)

  def count_followees(self, request_metadata, account_id):
    return self.rpc_wrapper(request_metadata.id, "follow:count_followees")(
        self._tclient.count_followees, request_metadata=request_metadata,
        account_id=account_id)
