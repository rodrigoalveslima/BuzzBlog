# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.account_client import Client as AccountClient

# Constants
IP_ADDRESS = "localhost"
ACCOUNT_PORT = 9090
NON_EXISTING_ACCOUNT_ID = -1


def random_id(size=16, chars=string.ascii_letters + string.digits):
  return ''.join(random.choice(chars) for _ in range(size))


class TestService(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super(TestService, self).__init__(*args, **kwargs)
    # Create test account.
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      self._account_passwd = "passwd"
      self._account = client.create_account(TRequestMetadata(id=random_id()),
                                            random_id(), self._account_passwd,
                                            "George", "Burdell")

  def test_authenticate_user(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create an account.
      account_passwd = "passwd"
      account = client.create_account(TRequestMetadata(id=random_id()),
                                      random_id(), account_passwd, "George",
                                      "Burdell")
      # Check that a wrong password is not authenticated.
      with self.assertRaises(TAccountInvalidCredentialsException):
        client.authenticate_user(
            TRequestMetadata(id=random_id(), requester_id=account.id),
            account.username, account_passwd + "123")
      # Authenticate user.
      authenticated_account = client.authenticate_user(
          TRequestMetadata(id=random_id(), requester_id=account.id),
          account.username, account_passwd)
      # Check the returned account's attributes.
      self.assertEqual(account.id, authenticated_account.id)
      self.assertEqual(account.created_at, authenticated_account.created_at)
      self.assertEqual(account.active, authenticated_account.active)
      self.assertEqual(account.username, authenticated_account.username)
      self.assertEqual(account.first_name, authenticated_account.first_name)
      self.assertEqual(account.last_name, authenticated_account.last_name)
      self.assertFalse(authenticated_account.followed_by_you)
      # Delete that account.
      client.delete_account(
          TRequestMetadata(id=random_id(), requester_id=account.id), account.id)
      # Check that the deleted account is not authenticated.
      with self.assertRaises(TAccountDeactivatedException):
        client.authenticate_user(
            TRequestMetadata(id=random_id(), requester_id=account.id),
            account.username, account_passwd)

  def test_create_account(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create an account.
      account = client.create_account(TRequestMetadata(id=random_id()),
                                      "gburdell", "passwd", "George", "Burdell")
      # Check the returned account's attributes.
      self.assertAlmostEqual(time.time(), account.created_at, delta=60)
      self.assertTrue(account.active)
      self.assertEqual("gburdell", account.username)
      self.assertEqual("George", account.first_name)
      self.assertEqual("Burdell", account.last_name)
      self.assertFalse(account.followed_by_you)
      # Check that attributes are being validated.
      with self.assertRaises(TAccountInvalidAttributesException):
        client.create_account(TRequestMetadata(id=random_id()),
                              "gburdell" * 128, "passwd", "George", "Burdell")
      # Check that an existing username cannot be used to create an account.
      with self.assertRaises(TAccountUsernameAlreadyExistsException):
        client.create_account(TRequestMetadata(id=random_id()), "gburdell",
                              "passwd", "George", "Burdell")

  def test_retrieve_standard_account(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Retrieve standard account and check its attributes.
      retrieved_account = client.retrieve_standard_account(
          TRequestMetadata(id=random_id(), requester_id=self._account.id),
          self._account.id)
      self.assertEqual(self._account.id, retrieved_account.id)
      self.assertEqual(self._account.created_at, retrieved_account.created_at)
      self.assertEqual(self._account.active, retrieved_account.active)
      self.assertEqual(self._account.username, retrieved_account.username)
      self.assertEqual(self._account.first_name, retrieved_account.first_name)
      self.assertEqual(self._account.last_name, retrieved_account.last_name)
      self.assertFalse(retrieved_account.followed_by_you)
      # Check that an account that does not exist is not retrieved.
      with self.assertRaises(TAccountNotFoundException):
        client.retrieve_standard_account(
            TRequestMetadata(id=random_id(), requester_id=self._account.id),
            NON_EXISTING_ACCOUNT_ID)

  def test_retrieve_expanded_account(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Retrieve expanded account and check its attributes.
      retrieved_account = client.retrieve_expanded_account(
          TRequestMetadata(id=random_id(), requester_id=self._account.id),
          self._account.id)
      self.assertEqual(self._account.id, retrieved_account.id)
      self.assertEqual(self._account.created_at, retrieved_account.created_at)
      self.assertEqual(self._account.active, retrieved_account.active)
      self.assertEqual(self._account.username, retrieved_account.username)
      self.assertEqual(self._account.first_name, retrieved_account.first_name)
      self.assertEqual(self._account.last_name, retrieved_account.last_name)
      self.assertFalse(retrieved_account.followed_by_you)
      self.assertFalse(retrieved_account.follows_you)
      self.assertEqual(0, retrieved_account.n_followers)
      self.assertEqual(0, retrieved_account.n_following)
      self.assertEqual(0, retrieved_account.n_posts)
      self.assertEqual(0, retrieved_account.n_likes)
      # Check that an account that does not exist is not retrieved.
      with self.assertRaises(TAccountNotFoundException):
        client.retrieve_expanded_account(
            TRequestMetadata(id=random_id(), requester_id=self._account.id),
            NON_EXISTING_ACCOUNT_ID)

  def test_update_account(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create an account.
      account = client.create_account(TRequestMetadata(id=random_id()),
                                      "john.doe", "passwd", "John", "Doe")
      # Update that account and check its attributes.
      updated_account = client.update_account(
          TRequestMetadata(id=random_id(), requester_id=account.id), account.id,
          "passwd", "Jane", "Doe")
      self.assertEqual(account.id, updated_account.id)
      self.assertEqual(account.created_at, updated_account.created_at)
      self.assertEqual(account.active, updated_account.active)
      self.assertEqual(account.username, updated_account.username)
      self.assertEqual("Jane", updated_account.first_name)
      self.assertEqual("Doe", updated_account.last_name)
      self.assertFalse(updated_account.followed_by_you)
      # Check that unauthorized user cannot update that account.
      with self.assertRaises(TAccountNotAuthorizedException):
        client.update_account(
            TRequestMetadata(id=random_id(), requester_id=self._account.id),
            account.id, "passwd", "John", "Doe")
      # Check that attributes are being validated.
      with self.assertRaises(TAccountInvalidAttributesException):
        client.update_account(
            TRequestMetadata(id=random_id(), requester_id=account.id),
            account.id, "passwd", "John" * 128, "Doe")

  def test_delete_account(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create an account.
      account = client.create_account(TRequestMetadata(id=random_id()),
                                      random_id(), "passwd", "George",
                                      "Burdell")
      # Check that unauthorized user cannot delete that account.
      with self.assertRaises(TAccountNotAuthorizedException):
        client.delete_account(
            TRequestMetadata(id=random_id(), requester_id=self._account.id),
            account.id)
      # Delete that account.
      client.delete_account(
          TRequestMetadata(id=random_id(), requester_id=account.id), account.id)
      # Check that an account that does not exist is not deleted.
      with self.assertRaises(TAccountNotFoundException):
        client.delete_account(
            TRequestMetadata(id=random_id(),
                             requester_id=NON_EXISTING_ACCOUNT_ID),
            NON_EXISTING_ACCOUNT_ID)

  def test_list_accounts(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create an account.
      account = client.create_account(TRequestMetadata(id=random_id()),
                                      random_id(), "passwd", "George",
                                      "Burdell")
      # Retrieve this account and check its id.
      query = TAccountQuery(username=account.username)
      limit = 10
      offset = 0
      retrieved_accounts = client.list_accounts(
          TRequestMetadata(id=random_id(), requester_id=self._account.id),
          query, limit, offset)
      self.assertEqual(1, len(retrieved_accounts))
      self.assertEqual(account.id, retrieved_accounts[0].id)


if __name__ == "__main__":
  unittest.main()
