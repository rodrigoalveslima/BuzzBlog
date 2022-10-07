# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.wordfilter_client import Client as WordfilterClient

# Constants
IP_ADDRESS = "localhost"
WORDFILTER_PORT = 9096


def random_id(size=16, chars=string.ascii_letters + string.digits):
  return ''.join(random.choice(chars) for _ in range(size))


class TestService(unittest.TestCase):

  def test_is_valid_word(self):
    with WordfilterClient(IP_ADDRESS, WORDFILTER_PORT) as client:
      self.assertFalse(
          client.is_valid_word(TRequestMetadata(id=random_id()), "corinthians"))
      self.assertTrue(
          client.is_valid_word(TRequestMetadata(id=random_id()), "foobar"))


if __name__ == "__main__":
  unittest.main()
