# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TWordfilterService
from buzzblog.base_client import BaseClient


class Client(BaseClient):

  def __init__(self, ip_address, port, timeout=30000, connection_pool=None):
    super().__init__(TWordfilterService.Client, ip_address, port, timeout,
                     connection_pool)

  def is_valid_word(self, request_metadata, word):
    return self._tclient.is_valid_word(request_metadata=request_metadata,
                                       word=word)
