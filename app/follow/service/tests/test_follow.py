# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.account_client import Client as AccountClient
from buzzblog.follow_client import Client as FollowClient

IP_ADDRESS = "localhost"
ACCOUNT_PORT = 9090
FOLLOW_PORT = 9091
NON_EXISTING_FOLLOW_ID = -1


def random_id(size=16, chars=string.ascii_letters + string.digits):
  return ''.join(random.choice(chars) for _ in range(size))


class TestService(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super(TestService, self).__init__(*args, **kwargs)
    # Create test accounts.
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      self._accounts = [
          client.create_account(TRequestMetadata(id=random_id()), random_id(),
                                "passwd", "George", "Burdell") for i in range(4)
      ]
    # Create test follow.
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      self._follow = client.follow_account(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._accounts[1].id)

  def test_follow_account(self):
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Follow account.
      follow = client.follow_account(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
          self._accounts[0].id)
      # Check the returned follow's attributes.
      self.assertAlmostEqual(time.time(), follow.created_at, delta=60)
      self.assertEqual(self._accounts[1].id, follow.follower_id)
      self.assertEqual(self._accounts[0].id, follow.followee_id)
      # Check that attributes are being validated.
      with self.assertRaises(TFollowInvalidAttributesException):
        client.follow_account(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
            self._accounts[1].id)
      # Check that a follow cannot be duplicated.
      with self.assertRaises(TFollowAlreadyExistsException):
        client.follow_account(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
            self._accounts[0].id)

  def test_retrieve_standard_follow(self):
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Retrieve standard follow and check its attributes.
      retrieved_follow = client.retrieve_standard_follow(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._follow.id)
      self.assertEqual(self._follow.id, retrieved_follow.id)
      self.assertEqual(self._follow.created_at, retrieved_follow.created_at)
      self.assertEqual(self._follow.follower_id, retrieved_follow.follower_id)
      self.assertEqual(self._follow.followee_id, retrieved_follow.followee_id)
      # Check that a follow that does not exist is not retrieved.
      with self.assertRaises(TFollowNotFoundException):
        client.retrieve_standard_follow(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_FOLLOW_ID)

  def test_retrieve_expanded_follow(self):
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Retrieve expanded follow and check its attributes.
      retrieved_follow = client.retrieve_expanded_follow(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._follow.id)
      self.assertEqual(self._follow.id, retrieved_follow.id)
      self.assertEqual(self._follow.created_at, retrieved_follow.created_at)
      self.assertEqual(self._follow.follower_id, retrieved_follow.follower_id)
      self.assertEqual(self._follow.followee_id, retrieved_follow.followee_id)
      self.assertEqual(self._follow.follower_id, retrieved_follow.follower.id)
      self.assertEqual(self._follow.followee_id, retrieved_follow.followee.id)
      # Check that a follow that does not exist is not retrieved.
      with self.assertRaises(TFollowNotFoundException):
        client.retrieve_expanded_follow(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_FOLLOW_ID)

  def test_delete_follow(self):
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Follow account.
      follow = client.follow_account(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          self._accounts[2].id)
      # Check that unauthorized user cannot delete that follow.
      with self.assertRaises(TFollowNotAuthorizedException):
        client.delete_follow(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[1].id),
            follow.id)
      # Delete that follow.
      client.delete_follow(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
          follow.id)
      # Check that a follow that does not exist is not deleted.
      with self.assertRaises(TFollowNotFoundException):
        client.delete_follow(
            TRequestMetadata(id=random_id(), requester_id=self._accounts[0].id),
            NON_EXISTING_FOLLOW_ID)

  def test_list_follows(self):
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Follow account.
      follow = client.follow_account(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[2].id),
          self._accounts[0].id)
      # Retrieve this follow and check its id.
      query = TFollowQuery(follower_id=self._accounts[2].id)
      limit = 10
      offset = 0
      retrieved_follows = client.list_follows(
          TRequestMetadata(id=random_id(), requester_id=self._accounts[2].id),
          query, limit, offset)
      self.assertEqual(1, len(retrieved_follows))
      self.assertEqual(follow.id, retrieved_follows[0].id)

  def test_check_follow(self):
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Check that the test follow exists.
      self.assertTrue(
          client.check_follow(
              TRequestMetadata(id=random_id(),
                               requester_id=self._accounts[0].id),
              self._follow.follower_id, self._follow.followee_id))
      # Check that a follow does not exist.
      self.assertFalse(
          client.check_follow(
              TRequestMetadata(id=random_id(),
                               requester_id=self._accounts[2].id),
              self._accounts[2].id, self._accounts[1].id))

  def test_count_followers(self):
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Check the number of followers.
      self.assertEqual(
          0,
          client.count_followers(
              TRequestMetadata(id=random_id(),
                               requester_id=self._accounts[3].id),
              self._accounts[3].id))

  def test_count_followees(self):
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Check the number of followees.
      self.assertEqual(
          0,
          client.count_followees(
              TRequestMetadata(id=random_id(),
                               requester_id=self._accounts[3].id),
              self._accounts[3].id))


if __name__ == "__main__":
  unittest.main()
