# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TAccountService
from buzzblog.base_client import BaseClient


class Client(BaseClient):

  def __init__(self, ip_address, port, timeout=30000, connection_pool=None):
    super().__init__(TAccountService.Client, ip_address, port, timeout,
                     connection_pool)

  def authenticate_user(self, request_metadata, username, password):
    return self._tclient.authenticate_user(request_metadata=request_metadata,
                                           username=username,
                                           password=password)

  def create_account(self, request_metadata, username, password, first_name,
                     last_name):
    return self._tclient.create_account(request_metadata=request_metadata,
                                        username=username,
                                        password=password,
                                        first_name=first_name,
                                        last_name=last_name)

  def retrieve_standard_account(self, request_metadata, account_id):
    return self._tclient.retrieve_standard_account(
        request_metadata=request_metadata, account_id=account_id)

  def retrieve_expanded_account(self, request_metadata, account_id):
    return self._tclient.retrieve_expanded_account(
        request_metadata=request_metadata, account_id=account_id)

  def update_account(self, request_metadata, account_id, password, first_name,
                     last_name):
    return self._tclient.update_account(request_metadata=request_metadata,
                                        account_id=account_id,
                                        password=password,
                                        first_name=first_name,
                                        last_name=last_name)

  def delete_account(self, request_metadata, account_id):
    return self._tclient.delete_account(request_metadata=request_metadata,
                                        account_id=account_id)

  def list_accounts(self, request_metadata, query, limit, offset):
    return self._tclient.list_accounts(request_metadata=request_metadata,
                                       query=query,
                                       limit=limit,
                                       offset=offset)
