# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.account_client import Client as AccountClient
from buzzblog.like_client import Client as LikeClient
from buzzblog.post_client import Client as PostClient

# Constants
IP_ADDRESS = "localhost"
ACCOUNT_PORT = 9090
LIKE_PORT = 9092
POST_PORT = 9093
NON_EXISTING_LIKE_ID = -1


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
    # Create test posts.
    with PostClient(IP_ADDRESS, POST_PORT) as client:
      self._posts = [
          client.create_post(
              TRequestMetadata(id=random_id(), requester_id=account.id),
              "Lorem ipsum") for account in self._accounts
      ]
    # Create test like.
    with LikeClient(IP_ADDRESS, LIKE_PORT) as client:
      self._like = client.like_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._posts[0].id)

  def test_like_post(self):
    with LikeClient(IP_ADDRESS, LIKE_PORT) as client:
      # Like a post.
      like = client.like_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._posts[1].id)
      # Check the returned like's attributes.
      self.assertAlmostEqual(time.time(), like.created_at, delta=60)
      self.assertEqual(self._accounts[0].id, like.account_id)
      self.assertEqual(self._posts[1].id, like.post_id)
      # Check that a like cannot be duplicated.
      with self.assertRaises(TLikeAlreadyExistsException):
        client.like_post(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            self._posts[1].id)

  def test_retrieve_standard_like(self):
    with LikeClient(IP_ADDRESS, LIKE_PORT) as client:
      # Retrieve standard like and check its attributes.
      retrieved_like = client.retrieve_standard_like(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._like.id)
      self.assertEqual(self._like.id, retrieved_like.id)
      self.assertEqual(self._like.created_at, retrieved_like.created_at)
      self.assertEqual(self._like.account_id, retrieved_like.account_id)
      self.assertEqual(self._like.post_id, retrieved_like.post_id)
      # Check that a like that does not exist is not retrieved.
      with self.assertRaises(TLikeNotFoundException):
        client.retrieve_standard_like(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_LIKE_ID)

  def test_retrieve_expanded_like(self):
    with LikeClient(IP_ADDRESS, LIKE_PORT) as client:
      # Retrieve expanded like and check its attributes.
      retrieved_like = client.retrieve_expanded_like(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._like.id)
      self.assertEqual(self._like.id, retrieved_like.id)
      self.assertEqual(self._like.created_at, retrieved_like.created_at)
      self.assertEqual(self._like.account_id, retrieved_like.account_id)
      self.assertEqual(self._like.post_id, retrieved_like.post_id)
      self.assertEqual(self._like.account_id, retrieved_like.account.id)
      self.assertEqual(self._like.post_id, retrieved_like.post.id)
      # Check that a like that does not exist is not retrieved.
      with self.assertRaises(TLikeNotFoundException):
        client.retrieve_expanded_like(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_LIKE_ID)

  def test_delete_like(self):
    with LikeClient(IP_ADDRESS, LIKE_PORT) as client:
      # Like a post.
      like = client.like_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._posts[2].id)
      # Check that unauthorized user cannot delete that like.
      with self.assertRaises(TLikeNotAuthorizedException):
        client.delete_like(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
            like.id)
      # Delete that like.
      client.delete_like(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          like.id)
      # Check that a like that does not exist is not deleted.
      with self.assertRaises(TLikeNotFoundException):
        client.delete_like(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_LIKE_ID)

  def test_list_likes(self):
    with LikeClient(IP_ADDRESS, LIKE_PORT) as client:
      # Like a post.
      like = client.like_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
          self._posts[0].id)
      # Retrieve this like and check its id.
      query = TLikeQuery(account_id=self._accounts[1].id)
      limit = 10
      offset = 0
      retrieved_likes = client.list_likes(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
          query, limit, offset)
      self.assertEqual(1, len(retrieved_likes))
      self.assertEqual(like.id, retrieved_likes[0].id)

  def test_count_likes_by_account(self):
    with LikeClient(IP_ADDRESS, LIKE_PORT) as client:
      # Like a post.
      like = client.like_post(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[2].id),
          self._posts[0].id)
      # Check the number of posts liked by that account.
      self.assertEqual(
          1,
          client.count_likes_by_account(
              TRequestMetadata(id=random_id(),
                               requester_id=self._accounts[2].id),
              self._accounts[2].id))

  def test_count_likes_of_post(self):
    with LikeClient(IP_ADDRESS, LIKE_PORT) as client:
      # Check the number of times a test post was liked.
      self.assertEqual(
          0,
          client.count_likes_of_post(
              TRequestMetadata(id=random_id(),
                               requester_id=self._accounts[0].id),
              self._posts[2].id))


if __name__ == "__main__":
  unittest.main()
