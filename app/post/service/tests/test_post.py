# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.account_client import Client as AccountClient
from buzzblog.post_client import Client as PostClient

# Constants
IP_ADDRESS = "localhost"
ACCOUNT_PORT = 9090
POST_PORT = 9093
NON_EXISTING_POST_ID = -1


def random_id(size=16, chars=string.ascii_letters + string.digits):
  return ''.join(random.choice(chars) for _ in range(size))


class TestService(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super(TestService, self).__init__(*args, **kwargs)
    # Create test accounts.
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      self._accounts = [
          client.create_account(TRequestMetadata(id=random_id()), random_id(),
                                "passwd", "George", "Burdell") for i in range(3)
      ]
    # Create test post.
    with PostClient(IP_ADDRESS, POST_PORT) as client:
      self._post = client.create_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          "Lorem ipsum")

  def test_create_post(self):
    with PostClient(IP_ADDRESS, POST_PORT) as client:
      # Create post.
      post = client.create_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          "dolor sit amet")
      # Check the returned post's attributes.
      self.assertAlmostEqual(time.time(), post.created_at, delta=60)
      self.assertTrue(post.active)
      self.assertEqual("dolor sit amet", post.text)
      self.assertEqual(self._accounts[0].id, post.author_id)
      # Check that attributes are being validated.
      with self.assertRaises(TPostInvalidAttributesException):
        client.create_post(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            "dolor sit amet" * 16)

  def test_retrieve_standard_post(self):
    with PostClient(IP_ADDRESS, POST_PORT) as client:
      # Retrieve standard post and check its attributes.
      retrieved_post = client.retrieve_standard_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._post.id)
      self.assertEqual(self._post.id, retrieved_post.id)
      self.assertEqual(self._post.created_at, retrieved_post.created_at)
      self.assertEqual(self._post.active, retrieved_post.active)
      self.assertEqual(self._post.text, retrieved_post.text)
      self.assertEqual(self._post.author_id, retrieved_post.author_id)
      # Check that a post that does not exist is not retrieved.
      with self.assertRaises(TPostNotFoundException):
        client.retrieve_standard_post(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_POST_ID)

  def test_retrieve_expanded_post(self):
    with PostClient(IP_ADDRESS, POST_PORT) as client:
      # Retrieve standard post and check its attributes.
      retrieved_post = client.retrieve_expanded_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._post.id)
      self.assertEqual(self._post.id, retrieved_post.id)
      self.assertEqual(self._post.created_at, retrieved_post.created_at)
      self.assertEqual(self._post.active, retrieved_post.active)
      self.assertEqual(self._post.text, retrieved_post.text)
      self.assertEqual(self._post.author_id, retrieved_post.author_id)
      self.assertEqual(self._post.author_id, retrieved_post.author.id)
      self.assertEqual(0, retrieved_post.n_likes)
      # Check that a post that does not exist is not retrieved.
      with self.assertRaises(TPostNotFoundException):
        client.retrieve_expanded_post(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_POST_ID)

  def test_delete_post(self):
    with PostClient(IP_ADDRESS, POST_PORT) as client:
      # Create post.
      post = client.create_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          "consectetur adipiscing elit")
      # Check that unauthorized user cannot delete that post.
      with self.assertRaises(TPostNotAuthorizedException):
        client.delete_post(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
            post.id)
      # Delete that post.
      client.delete_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          post.id)
      # Check that a post that does not exist is not deleted.
      with self.assertRaises(TPostNotFoundException):
        client.delete_post(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_POST_ID)

  def test_list_posts(self):
    with PostClient(IP_ADDRESS, POST_PORT) as client:
      # Create post.
      post = client.create_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
          "Lorem ipsum")
      # Retrieve this post and check its id.
      query = TPostQuery(author_id=self._accounts[1].id)
      limit = 10
      offset = 0
      retrieved_posts = client.list_posts(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
          query, limit, offset)
      self.assertEqual(1, len(retrieved_posts))
      self.assertEqual(post.id, retrieved_posts[0].id)

  def test_count_posts_by_author(self):
    with PostClient(IP_ADDRESS, POST_PORT) as client:
      # Check the number of posts.
      self.assertEqual(
          0,
          client.count_posts_by_author(
              TRequestMetadata(id=random_id(),
                               requester_id=self._accounts[2].id),
              self._accounts[2].id))


if __name__ == "__main__":
  unittest.main()
