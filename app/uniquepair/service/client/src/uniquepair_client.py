# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

from buzzblog.gen import TUniquepairService
from buzzblog.base_client import BaseClient


class Client(BaseClient):

  def __init__(self, ip_address, port, timeout=30000, connection_pool=None):
    super().__init__(TUniquepairService.Client, ip_address, port, timeout,
                     connection_pool)

  def get(self, request_metadata, uniquepair_id):
    return self._tclient.get(request_metadata=request_metadata,
                             uniquepair_id=uniquepair_id)

  def add(self, request_metadata, domain, first_elem, second_elem):
    return self._tclient.add(request_metadata=request_metadata,
                             domain=domain,
                             first_elem=first_elem,
                             second_elem=second_elem)

  def remove(self, request_metadata, uniquepair_id):
    return self._tclient.remove(request_metadata=request_metadata,
                                uniquepair_id=uniquepair_id)

  def find(self, request_metadata, domain, first_elem, second_elem):
    return self._tclient.find(request_metadata=request_metadata,
                              domain=domain,
                              first_elem=first_elem,
                              second_elem=second_elem)

  def fetch(self, request_metadata, query, limit, offset):
    return self._tclient.fetch(request_metadata=request_metadata,
                               query=query,
                               limit=limit,
                               offset=offset)

  def count(self, request_metadata, query):
    return self._tclient.count(request_metadata=request_metadata, query=query)
