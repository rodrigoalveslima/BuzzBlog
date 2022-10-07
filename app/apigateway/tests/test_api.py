# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import time
import unittest

import requests
from requests.auth import HTTPBasicAuth

# Constants
SERVER_HOSTNAME = "localhost"
SERVER_PORT = 8080
URL = "{hostname}:{port}".format(hostname=SERVER_HOSTNAME, port=SERVER_PORT)


def random_id(size=16, chars=string.ascii_letters + string.digits):
  return ''.join(random.choice(chars) for _ in range(size))


class TestService(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super(TestService, self).__init__(*args, **kwargs)
    # Create test accounts.
    self._accounts = [{
        "username": random_id(),
        "password": "passwd",
        "first_name": "George",
        "last_name": "Burdell"
    } for i in range(4)]
    for account in self._accounts:
      r = requests.post("http://{url}/account".format(url=URL),
                        params={"request_id": random_id()},
                        json={
                            "username": account["username"],
                            "password": account["password"],
                            "first_name": account["first_name"],
                            "last_name": account["last_name"]
                        })
      response = r.json()
      account["id"] = response["id"]
    # Create test follow.
    self._follow = {
        "follower": self._accounts[0],
        "followee": self._accounts[1]
    }
    r = requests.post("http://{url}/follow".format(url=URL),
                      auth=HTTPBasicAuth(self._follow["follower"]["username"],
                                         self._follow["follower"]["password"]),
                      params={"request_id": random_id()},
                      json={"account_id": self._follow["followee"]["id"]})
    response = r.json()
    self._follow["id"] = response["id"]
    # Create test posts.
    self._posts = [{
        "text": "Lorem ipsum",
        "author": self._accounts[0]
    } for i in range(2)]
    for post in self._posts:
      r = requests.post("http://{url}/post".format(url=URL),
                        auth=HTTPBasicAuth(post["author"]["username"],
                                           post["author"]["password"]),
                        params={"request_id": random_id()},
                        json={"text": post["text"]})
      response = r.json()
      post["id"] = response["id"]
    # Create test like.
    self._like = {"account": self._accounts[1], "post": self._posts[0]}
    r = requests.post("http://{url}/like".format(url=URL),
                      auth=HTTPBasicAuth(self._like["account"]["username"],
                                         self._like["account"]["password"]),
                      params={"request_id": random_id()},
                      json={"post_id": self._like["post"]["id"]})
    response = r.json()
    self._like["id"] = response["id"]

  def test_create_account_200(self):
    r = requests.post("http://{url}/account".format(url=URL),
                      params={"request_id": random_id()},
                      json={
                          "username": "jane.doe",
                          "password": "passwd",
                          "first_name": "Jane",
                          "last_name": "Doe"
                      })
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("account", response["object"])
    self.assertEqual("standard", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertTrue(response["active"])
    self.assertEqual("jane.doe", response["username"])
    self.assertEqual("Jane", response["first_name"])
    self.assertEqual("Doe", response["last_name"])

  def test_retrieve_account_200(self):
    r = requests.get("http://{url}/account/{account_id}".format(
        url=URL, account_id=self._accounts[3]["id"]),
                     params={"request_id": random_id()})
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("account", response["object"])
    self.assertEqual("expanded", response["mode"])
    self.assertEqual(self._accounts[3]["id"], response["id"])
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertTrue(response["active"])
    self.assertEqual(self._accounts[3]["username"], response["username"])
    self.assertEqual(self._accounts[3]["first_name"], response["first_name"])
    self.assertEqual(self._accounts[3]["last_name"], response["last_name"])
    self.assertFalse(response["follows_you"])
    self.assertFalse(response["followed_by_you"])
    self.assertEqual(0, response["n_followers"])
    self.assertEqual(0, response["n_following"])
    self.assertEqual(0, response["n_posts"])
    self.assertEqual(0, response["n_likes"])

  def test_update_account_200(self):
    self._accounts[0]["last_name"] = "P. Burdell"
    r = requests.put("http://{url}/account/{account_id}".format(
        url=URL, account_id=self._accounts[0]["id"]),
                     auth=HTTPBasicAuth(self._accounts[0]["username"],
                                        self._accounts[0]["password"]),
                     params={"request_id": random_id()},
                     json={
                         "password": self._accounts[0]["password"],
                         "first_name": self._accounts[0]["first_name"],
                         "last_name": self._accounts[0]["last_name"]
                     })
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("account", response["object"])
    self.assertEqual("standard", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertTrue(response["active"])
    self.assertEqual(self._accounts[0]["username"], response["username"])
    self.assertEqual(self._accounts[0]["first_name"], response["first_name"])
    self.assertEqual(self._accounts[0]["last_name"], response["last_name"])

  def test_delete_account_200(self):
    # Create an account.
    r = requests.post("http://{url}/account".format(url=URL),
                      params={"request_id": random_id()},
                      json={
                          "username": random_id(),
                          "password": "passwd",
                          "first_name": "George",
                          "last_name": "Burdell"
                      })
    response = r.json()
    # Delete that account.
    r = requests.delete("http://{url}/account/{account_id}".format(
        url=URL, account_id=response["id"]),
                        auth=HTTPBasicAuth(response["username"], "passwd"),
                        params={"request_id": random_id()})
    self.assertEqual(200, r.status_code)

  def test_list_accounts_200(self):
    r = requests.get("http://{url}/account".format(url=URL),
                     params={
                         "request_id": random_id(),
                         "username": random_id()
                     },
                     json={
                         "limit": 10,
                         "offset": 0
                     })
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual(0, len(response))

  def test_follow_account_200(self):
    r = requests.post("http://{url}/follow".format(url=URL),
                      auth=HTTPBasicAuth(self._accounts[1]["username"],
                                         self._accounts[1]["password"]),
                      params={"request_id": random_id()},
                      json={"account_id": self._accounts[0]["id"]})
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("follow", response["object"])
    self.assertEqual("standard", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertEqual(self._accounts[1]["id"], response["follower_id"])
    self.assertEqual(self._accounts[0]["id"], response["followee_id"])

  def test_retrieve_follow_200(self):
    r = requests.get("http://{url}/follow/{follow_id}".format(
        url=URL, follow_id=self._follow["id"]),
                     params={"request_id": random_id()})
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("follow", response["object"])
    self.assertEqual("expanded", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertEqual(self._follow["follower"]["id"], response["follower_id"])
    self.assertEqual(self._follow["followee"]["id"], response["followee_id"])
    self.assertEqual(self._follow["follower"]["id"], response["follower"]["id"])
    self.assertEqual(self._follow["followee"]["id"], response["followee"]["id"])

  def test_delete_follow_200(self):
    # Follow an account.
    r = requests.post("http://{url}/follow".format(url=URL),
                      auth=HTTPBasicAuth(self._accounts[0]["username"],
                                         self._accounts[0]["password"]),
                      params={"request_id": random_id()},
                      json={"account_id": self._accounts[2]["id"]})
    response = r.json()
    # Delete that follow.
    r = requests.delete("http://{url}/follow/{follow_id}".format(
        url=URL, follow_id=response["id"]),
                        auth=HTTPBasicAuth(self._accounts[0]["username"],
                                           self._accounts[0]["password"]),
                        params={"request_id": random_id()})
    self.assertEqual(200, r.status_code)

  def test_list_follows_200(self):
    r = requests.get("http://{url}/follow".format(url=URL),
                     params={
                         "request_id": random_id(),
                         "follower_id": self._accounts[3]["id"]
                     },
                     json={
                         "limit": 10,
                         "offset": 0
                     })
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual(0, len(response))

  def test_create_post_200(self):
    r = requests.post("http://{url}/post".format(url=URL),
                      auth=HTTPBasicAuth(self._accounts[0]["username"],
                                         self._accounts[0]["password"]),
                      params={"request_id": random_id()},
                      json={"text": "Lorem ipsum"})
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("post", response["object"])
    self.assertEqual("standard", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertTrue(response["active"])
    self.assertEqual("Lorem ipsum", response["text"])
    self.assertEqual(self._accounts[0]["id"], response["author_id"])

  def test_retrieve_post_200(self):
    r = requests.get("http://{url}/post/{post_id}".format(
        url=URL, post_id=self._posts[0]["id"]),
                     params={"request_id": random_id()})
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("post", response["object"])
    self.assertEqual("expanded", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertTrue(response["active"])
    self.assertEqual(self._posts[0]["text"], response["text"])
    self.assertEqual(self._posts[0]["author"]["id"], response["author_id"])
    self.assertEqual(self._posts[0]["author"]["id"], response["author"]["id"])

  def test_delete_post_200(self):
    # Create a post.
    r = requests.post("http://{url}/post".format(url=URL),
                      auth=HTTPBasicAuth(self._accounts[0]["username"],
                                         self._accounts[0]["password"]),
                      params={"request_id": random_id()},
                      json={"text": "Lorem ipsum"})
    response = r.json()
    # Delete that post.
    r = requests.delete("http://{url}/post/{post_id}".format(
        url=URL, post_id=response["id"]),
                        auth=HTTPBasicAuth(self._accounts[0]["username"],
                                           self._accounts[0]["password"]),
                        params={"request_id": random_id()})
    self.assertEqual(200, r.status_code)

  def test_list_posts_200(self):
    r = requests.get("http://{url}/post".format(url=URL),
                     params={
                         "request_id": random_id(),
                         "author_id": self._accounts[3]["id"]
                     },
                     json={
                         "limit": 10,
                         "offset": 0
                     })
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual(0, len(response))

  def test_like_post_200(self):
    r = requests.post("http://{url}/like".format(url=URL),
                      auth=HTTPBasicAuth(self._accounts[0]["username"],
                                         self._accounts[0]["password"]),
                      params={"request_id": random_id()},
                      json={"post_id": self._posts[1]["id"]})
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("like", response["object"])
    self.assertEqual("standard", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertEqual(self._accounts[0]["id"], response["account_id"])
    self.assertEqual(self._posts[1]["id"], response["post_id"])

  def test_retrieve_like_200(self):
    r = requests.get("http://{url}/like/{like_id}".format(
        url=URL, like_id=self._like["id"]),
                     params={"request_id": random_id()})
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("like", response["object"])
    self.assertEqual("expanded", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertEqual(self._like["account"]["id"], response["account_id"])
    self.assertEqual(self._like["post"]["id"], response["post_id"])
    self.assertEqual(self._like["account"]["id"], response["account"]["id"])
    self.assertEqual(self._like["post"]["id"], response["post"]["id"])

  def test_delete_like_200(self):
    # Like a post.
    r = requests.post("http://{url}/like".format(url=URL),
                      auth=HTTPBasicAuth(self._accounts[0]["username"],
                                         self._accounts[0]["password"]),
                      params={"request_id": random_id()},
                      json={"post_id": self._posts[0]["id"]})
    response = r.json()
    # Delete that like.
    r = requests.delete("http://{url}/like/{like_id}".format(
        url=URL, like_id=response["id"]),
                        auth=HTTPBasicAuth(self._accounts[0]["username"],
                                           self._accounts[0]["password"]),
                        params={"request_id": random_id()})
    self.assertEqual(200, r.status_code)

  def test_list_likes_200(self):
    r = requests.get("http://{url}/like".format(url=URL),
                     params={
                         "request_id": random_id(),
                         "account_id": self._accounts[3]["id"]
                     },
                     json={
                         "limit": 10,
                         "offset": 0
                     })
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual(0, len(response))

  def test_list_trending_hashtags_200(self):
    r = requests.get("http://{url}/trending".format(url=URL),
                     params={"request_id": random_id()},
                     json={"limit": 10})
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertIsInstance(response, list)


if __name__ == "__main__":
  unittest.main()
