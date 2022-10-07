# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.trending_client import Client as TrendingClient

# Constants
IP_ADDRESS = "localhost"
TRENDING_PORT = 9095


def random_id(size=16, chars=string.ascii_letters + string.digits):
  return ''.join(random.choice(chars) for _ in range(size))


class TestService(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super(TestService, self).__init__(*args, **kwargs)
    # Add hashtags.
    with TrendingClient(IP_ADDRESS, TRENDING_PORT) as client:
      client.process_post(TRequestMetadata(id=random_id()),
                          "#Lorem ipsum dolor sit amet")
      client.process_post(TRequestMetadata(id=random_id()),
                          "Lorem #ipsum dolor sit amet")
      client.process_post(TRequestMetadata(id=random_id()),
                          "Lorem ipsum #dolor #sit amet")
      client.process_post(TRequestMetadata(id=random_id()),
                          "#Lorem ipsum dolor sit amet")

  def test_process_post(self):
    with TrendingClient(IP_ADDRESS, TRENDING_PORT) as client:
      # Process posts.
      client.process_post(TRequestMetadata(id=random_id()),
                          "Palmeiras nao tem mundial")
      client.process_post(TRequestMetadata(id=random_id()),
                          "#Palmeiras #nao #tem #mundial")
      client.process_post(TRequestMetadata(id=random_id()),
                          "#Palmeiras nao tem #mundial")
      client.process_post(TRequestMetadata(id=random_id()),
                          "#Palmeiras nao tem mundial")
      client.process_post(TRequestMetadata(id=random_id()),
                          "Palmeiras nao tem #mundial")

  def test_fetch_trending_hashtags(self):
    with TrendingClient(IP_ADDRESS, TRENDING_PORT) as client:
      # Fetch top 10 trending hashtags.
      trending_hashtags = client.fetch_trending_hashtags(
          TRequestMetadata(id=random_id()), 10)
      # Check hashtags added during initialization.
      self.assertIn("Lorem", trending_hashtags)
      self.assertIn("ipsum", trending_hashtags)
      self.assertIn("dolor", trending_hashtags)
      self.assertIn("sit", trending_hashtags)


if __name__ == "__main__":
  unittest.main()
