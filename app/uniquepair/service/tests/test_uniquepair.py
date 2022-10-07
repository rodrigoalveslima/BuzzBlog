# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.uniquepair_client import Client as UniquepairClient

# Constants
IP_ADDRESS = "localhost"
UNIQUEPAIR_PORT = 9094
NON_EXISTING_UNIQUEPAIR_ID = -1
TEST_DOMAIN = "test"


def random_id(size=16, chars=string.ascii_letters + string.digits):
  return ''.join(random.choice(chars) for _ in range(size))


def random_int():
  return random.randint(1, 2**16)


class TestService(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super(TestService, self).__init__(*args, **kwargs)
    # Add test unique pair.
    with UniquepairClient(IP_ADDRESS, UNIQUEPAIR_PORT) as client:
      self._uniquepair = client.add(TRequestMetadata(id=random_id()),
                                    TEST_DOMAIN, random_int(), random_int())

  def test_get(self):
    with UniquepairClient(IP_ADDRESS, UNIQUEPAIR_PORT) as client:
      # Get unique pair and check its attributes.
      uniquepair = client.get(TRequestMetadata(id=random_id()),
                              self._uniquepair.id)
      self.assertEqual(self._uniquepair.id, uniquepair.id)
      self.assertEqual(self._uniquepair.created_at, uniquepair.created_at)
      self.assertEqual(self._uniquepair.domain, uniquepair.domain)
      self.assertEqual(self._uniquepair.first_elem, uniquepair.first_elem)
      self.assertEqual(self._uniquepair.second_elem, uniquepair.second_elem)
      # Check that a unique pair that does not exist is not retrieved.
      with self.assertRaises(TUniquepairNotFoundException):
        client.get(TRequestMetadata(id=random_id()), NON_EXISTING_UNIQUEPAIR_ID)

  def test_add(self):
    with UniquepairClient(IP_ADDRESS, UNIQUEPAIR_PORT) as client:
      # Add unique pair.
      first_elem = random_int()
      second_elem = random_int()
      uniquepair = client.add(TRequestMetadata(id=random_id()), TEST_DOMAIN,
                              first_elem, second_elem)
      # Check the returned unique pair's attributes.
      self.assertAlmostEqual(time.time(), uniquepair.created_at, delta=60)
      self.assertEqual(TEST_DOMAIN, uniquepair.domain)
      self.assertEqual(first_elem, uniquepair.first_elem)
      self.assertEqual(second_elem, uniquepair.second_elem)
      # Check that an existing pair cannot be added.
      with self.assertRaises(TUniquepairAlreadyExistsException):
        client.add(TRequestMetadata(id=random_id()), TEST_DOMAIN, first_elem,
                   second_elem)

  def test_remove(self):
    with UniquepairClient(IP_ADDRESS, UNIQUEPAIR_PORT) as client:
      # Add unique pair.
      uniquepair = client.add(TRequestMetadata(id=random_id()), TEST_DOMAIN,
                              random_int(), random_int())
      # Remove that unique pair.
      client.remove(TRequestMetadata(id=random_id()), uniquepair.id)
      # Check that a unique pair that does not exist is not removed.
      with self.assertRaises(TUniquepairNotFoundException):
        client.remove(TRequestMetadata(id=random_id()),
                      NON_EXISTING_UNIQUEPAIR_ID)

  def test_find(self):
    with UniquepairClient(IP_ADDRESS, UNIQUEPAIR_PORT) as client:
      # Check that a unique pair that exists is found.
      self.assertTrue(
          client.find(TRequestMetadata(id=random_id()), TEST_DOMAIN,
                      self._uniquepair.first_elem,
                      self._uniquepair.second_elem))
      # Check that a unique pair that does not exist is not found.
      self.assertFalse(
          client.find(TRequestMetadata(id=random_id()), TEST_DOMAIN, 42, 42))

  def test_fetch(self):
    with UniquepairClient(IP_ADDRESS, UNIQUEPAIR_PORT) as client:
      # Fetch unique pair and check its id.
      query = TUniquepairQuery(domain=TEST_DOMAIN,
                               first_elem=self._uniquepair.first_elem,
                               second_elem=self._uniquepair.second_elem)
      limit = 10
      offset = 0
      uniquepairs = client.fetch(TRequestMetadata(id=random_id()), query, limit,
                                 offset)
      self.assertEqual(1, len(uniquepairs))
      self.assertEqual(self._uniquepair.id, uniquepairs[0].id)

  def test_count(self):
    with UniquepairClient(IP_ADDRESS, UNIQUEPAIR_PORT) as client:
      # Check the number of unique pairs.
      query = TUniquepairQuery(domain=TEST_DOMAIN,
                               first_elem=self._uniquepair.first_elem,
                               second_elem=self._uniquepair.second_elem)
      self.assertEqual(1, client.count(TRequestMetadata(id=random_id()), query))


if __name__ == "__main__":
  unittest.main()
