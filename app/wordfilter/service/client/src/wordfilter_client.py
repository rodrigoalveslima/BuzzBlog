# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TWordfilterService
from buzzblog.base_client import BaseClient


class Client(BaseClient):
  def __init__(self, ip_address, port, timeout=30000, logger=None, connection_pool=None):
    super().__init__(TWordfilterService.Client, ip_address, port, timeout,
        logger, connection_pool)

  def is_valid_word(self, request_metadata, word):
    return self.rpc_wrapper(request_metadata.id, "wordfilter:is_valid_word")(
        self._tclient.is_valid_word, request_metadata=request_metadata,
        word=word)
