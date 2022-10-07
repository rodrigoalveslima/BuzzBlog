# Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
# Systems

import os

import flask
import flask_httpauth
import spdlog as spd

from buzzblog.gen.ttypes import *
from buzzblog.microservice_connected_server import MicroserviceConnectedServer
from buzzblog.utils import RPC_WRAPPER


def setup_app():
  app = flask.Flask(__name__)
  app.url_map.strict_slashes = False
  app.rpc = MicroserviceConnectedServer(
      "apigateway", "/etc/opt/BuzzBlog/backend.yml",
      int(os.getenv("microservice_connection_pool_min_size")),
      int(os.getenv("microservice_connection_pool_max_size")),
      int(os.getenv("microservice_connection_pool_allow_ephemeral")) == 1,
      int(os.getenv("logging")))
  if int(os.getenv("logging")):
    app.rpc_logger = spd.FileLogger("rpc_logger_%s" % os.getpid(),
                                    "/tmp/rpc_%s.log" % os.getpid())
    app.rpc_logger.set_pattern("[%Y-%m-%d %H:%M:%S.%f] pid=%P tid=%t %v")
  else:
    app.rpc_logger = None
  return app


app = setup_app()
auth = flask_httpauth.HTTPBasicAuth()


@auth.verify_password
def verify_password(username, password):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  try:
    account = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=verify_password rs=account rf=authenticate_user rid=%s"
        % request_metadata.id)(app.rpc.authenticate_user,
                               request_metadata=request_metadata,
                               username=username,
                               password=password)
  except:
    account = None
  return account


@app.route("/account", methods=["POST"])
def create_account():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  params = flask.request.get_json()
  try:
    username = params["username"]
    password = params["password"]
    first_name = params["first_name"]
    last_name = params["last_name"]
  except KeyError:
    return ({}, 400)
  try:
    account = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=create_account rs=account rf=create_account rid=%s" %
        request_metadata.id)(app.rpc.create_account,
                             request_metadata=request_metadata,
                             username=username,
                             password=password,
                             first_name=first_name,
                             last_name=last_name)
  except TAccountInvalidAttributesException:
    return ({}, 400)
  except TAccountUsernameAlreadyExistsException:
    return ({}, 400)
  return {
      "object": "account",
      "mode": "standard",
      "id": account.id,
      "created_at": account.created_at,
      "active": account.active,
      "username": account.username,
      "first_name": account.first_name,
      "last_name": account.last_name
  }


@app.route("/account/<int:account_id>", methods=["GET"])
def retrieve_account(account_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  try:
    account = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=retrieve_account rs=account rf=retrieve_expanded_account rid=%s"
        % request_metadata.id)(app.rpc.retrieve_expanded_account,
                               request_metadata=request_metadata,
                               account_id=account_id)
  except TAccountNotFoundException:
    return ({}, 404)
  return {
      "object": "account",
      "mode": "expanded",
      "id": account.id,
      "created_at": account.created_at,
      "active": account.active,
      "username": account.username,
      "first_name": account.first_name,
      "last_name": account.last_name,
      "follows_you": account.follows_you,
      "followed_by_you": account.followed_by_you,
      "n_followers": account.n_followers,
      "n_following": account.n_following,
      "n_posts": account.n_posts,
      "n_likes": account.n_likes
  }


@app.route("/account/<int:account_id>", methods=["PUT"])
@auth.login_required
def update_account(account_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
                                      requester_id=auth.current_user().id)
  params = flask.request.get_json()
  try:
    password = params["password"]
    first_name = params["first_name"]
    last_name = params["last_name"]
  except KeyError:
    return ({}, 400)
  try:
    account = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=update_account rs=account rf=update_account rid=%s" %
        request_metadata.id)(app.rpc.update_account,
                             request_metadata=request_metadata,
                             account_id=account_id,
                             password=password,
                             first_name=first_name,
                             last_name=last_name)
  except TAccountInvalidAttributesException:
    return ({}, 400)
  except TAccountNotAuthorizedException:
    return ({}, 403)
  except TAccountNotFoundException:
    return ({}, 404)
  return {
      "object": "account",
      "mode": "standard",
      "id": account.id,
      "created_at": account.created_at,
      "active": account.active,
      "username": account.username,
      "first_name": account.first_name,
      "last_name": account.last_name
  }


@app.route("/account/<int:account_id>", methods=["DELETE"])
@auth.login_required
def delete_account(account_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
                                      requester_id=auth.current_user().id)
  try:
    RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=delete_account rs=account rf=delete_account rid=%s" %
        request_metadata.id)(app.rpc.delete_account,
                             request_metadata=request_metadata,
                             account_id=account_id)
  except TAccountNotAuthorizedException:
    return ({}, 403)
  except TAccountNotFoundException:
    return ({}, 404)
  return {}


@app.route("/account", methods=["GET"])
def list_accounts():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  limit = int(flask.request.args["limit"]) \
      if "limit" in flask.request.args else 32
  offset = int(flask.request.args["offset"]) \
      if "offset" in flask.request.args else 0
  username = flask.request.args["username"] \
      if "username" in flask.request.args else None
  query = TAccountQuery(username=username)
  accounts = RPC_WRAPPER(
      app.rpc_logger,
      "ls=apigateway lf=list_accounts rs=account rf=list_accounts rid=%s" %
      request_metadata.id)(app.rpc.list_accounts,
                           request_metadata=request_metadata,
                           query=query,
                           limit=limit,
                           offset=offset)
  return flask.jsonify([{
      "object": "account",
      "mode": "expanded",
      "id": account.id,
      "created_at": account.created_at,
      "active": account.active,
      "username": account.username,
      "first_name": account.first_name,
      "last_name": account.last_name,
      "follows_you": account.follows_you,
      "followed_by_you": account.followed_by_you,
      "n_followers": account.n_followers,
      "n_following": account.n_following,
      "n_posts": account.n_posts,
      "n_likes": account.n_likes
  } for account in accounts])


@app.route("/follow", methods=["POST"])
@auth.login_required
def follow_account():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
                                      requester_id=auth.current_user().id)
  params = flask.request.get_json()
  try:
    account_id = params["account_id"]
  except KeyError:
    return ({}, 400)
  try:
    follow = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=follow_account rs=follow rf=follow_account rid=%s" %
        request_metadata.id)(app.rpc.follow_account,
                             request_metadata=request_metadata,
                             account_id=account_id)
  except TFollowAlreadyExistsException:
    return ({}, 400)
  return {
      "object": "follow",
      "mode": "standard",
      "id": follow.id,
      "created_at": follow.created_at,
      "follower_id": follow.follower_id,
      "followee_id": follow.followee_id
  }


@app.route("/follow/<int:follow_id>", methods=["GET"])
def retrieve_follow(follow_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  try:
    follow = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=retrieve_follow rs=follow rf=retrieve_expanded_follow rid=%s"
        % request_metadata.id)(app.rpc.retrieve_expanded_follow,
                               request_metadata=request_metadata,
                               follow_id=follow_id)
  except TFollowNotFoundException:
    return ({}, 404)
  return {
      "object": "follow",
      "mode": "expanded",
      "id": follow.id,
      "created_at": follow.created_at,
      "follower_id": follow.follower_id,
      "followee_id": follow.followee_id,
      "follower": {
          "object": "account",
          "mode": "standard",
          "id": follow.follower.id,
          "created_at": follow.follower.created_at,
          "active": follow.follower.active,
          "username": follow.follower.username,
          "first_name": follow.follower.first_name,
          "last_name": follow.follower.last_name
      },
      "followee": {
          "object": "account",
          "mode": "standard",
          "id": follow.followee.id,
          "created_at": follow.followee.created_at,
          "active": follow.followee.active,
          "username": follow.followee.username,
          "first_name": follow.followee.first_name,
          "last_name": follow.followee.last_name
      }
  }


@app.route("/follow/<int:follow_id>", methods=["DELETE"])
@auth.login_required
def delete_follow(follow_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
                                      requester_id=auth.current_user().id)
  try:
    RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=delete_follow rs=follow rf=delete_follow rid=%s" %
        request_metadata.id)(app.rpc.delete_follow,
                             request_metadata=request_metadata,
                             follow_id=follow_id)
  except TFollowNotAuthorizedException:
    return ({}, 403)
  except TFollowNotFoundException:
    return ({}, 404)
  return {}


@app.route("/follow", methods=["GET"])
def list_follows():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  limit = int(flask.request.args["limit"]) \
      if "limit" in flask.request.args else 32
  offset = int(flask.request.args["offset"]) \
      if "offset" in flask.request.args else 0
  follower_id = int(flask.request.args["follower_id"]) \
      if "follower_id" in flask.request.args else None
  followee_id = int(flask.request.args["followee_id"]) \
      if "followee_id" in flask.request.args else None
  query = TFollowQuery(follower_id=follower_id, followee_id=followee_id)
  try:
    follows = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=list_follows rs=follow rf=list_follows rid=%s" %
        request_metadata.id)(app.rpc.list_follows,
                             request_metadata=request_metadata,
                             query=query,
                             limit=limit,
                             offset=offset)
  except TAccountNotFoundException:
    return ({}, 400)
  return flask.jsonify([{
      "object": "follow",
      "mode": "expanded",
      "id": follow.id,
      "created_at": follow.created_at,
      "follower_id": follow.follower_id,
      "followee_id": follow.followee_id,
      "follower": {
          "object": "account",
          "mode": "standard",
          "id": follow.follower.id,
          "created_at": follow.follower.created_at,
          "active": follow.follower.active,
          "username": follow.follower.username,
          "first_name": follow.follower.first_name,
          "last_name": follow.follower.last_name
      },
      "followee": {
          "object": "account",
          "mode": "standard",
          "id": follow.followee.id,
          "created_at": follow.followee.created_at,
          "active": follow.followee.active,
          "username": follow.followee.username,
          "first_name": follow.followee.first_name,
          "last_name": follow.followee.last_name
      }
  } for follow in follows])


@app.route("/post", methods=["POST"])
@auth.login_required
def create_post():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
                                      requester_id=auth.current_user().id)
  params = flask.request.get_json()
  try:
    text = params["text"]
  except KeyError:
    return ({}, 400)
  try:
    post = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=create_post rs=post rf=create_post rid=%s" %
        request_metadata.id)(app.rpc.create_post,
                             request_metadata=request_metadata,
                             text=text)
  except TPostInvalidAttributesException:
    return ({}, 400)
  return {
      "object": "post",
      "mode": "standard",
      "id": post.id,
      "created_at": post.created_at,
      "active": post.active,
      "text": post.text,
      "author_id": post.author_id
  }


@app.route("/post/<int:post_id>", methods=["GET"])
def retrieve_post(post_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  try:
    post = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=retrieve_post rs=post rf=retrieve_expanded_post rid=%s"
        % request_metadata.id)(app.rpc.retrieve_expanded_post,
                               request_metadata=request_metadata,
                               post_id=post_id)
  except TPostNotFoundException:
    return ({}, 404)
  return {
      "object": "post",
      "mode": "expanded",
      "id": post.id,
      "created_at": post.created_at,
      "active": post.active,
      "text": post.text,
      "author_id": post.author_id,
      "author": {
          "object": "account",
          "mode": "standard",
          "id": post.author.id,
          "created_at": post.author.created_at,
          "active": post.author.active,
          "username": post.author.username,
          "first_name": post.author.first_name,
          "last_name": post.author.last_name
      },
      "n_likes": post.n_likes
  }


@app.route("/post/<int:post_id>", methods=["DELETE"])
@auth.login_required
def delete_post(post_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
                                      requester_id=auth.current_user().id)
  try:
    RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=delete_post rs=post rf=delete_post rid=%s" %
        request_metadata.id)(app.rpc.delete_post,
                             request_metadata=request_metadata,
                             post_id=post_id)
  except TPostNotAuthorizedException:
    return ({}, 403)
  except TPostNotFoundException:
    return ({}, 404)
  return {}


@app.route("/post", methods=["GET"])
def list_posts():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  limit = int(flask.request.args["limit"]) \
      if "limit" in flask.request.args else 32
  offset = int(flask.request.args["offset"]) \
      if "offset" in flask.request.args else 0
  author_id = int(flask.request.args["author_id"]) \
      if "author_id" in flask.request.args else None
  query = TPostQuery(author_id=author_id)
  try:
    posts = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=list_posts rs=post rf=list_posts rid=%s" %
        request_metadata.id)(app.rpc.list_posts,
                             request_metadata=request_metadata,
                             query=query,
                             limit=limit,
                             offset=offset)
  except TAccountNotFoundException:
    return ({}, 400)
  return flask.jsonify([{
      "object": "post",
      "mode": "expanded",
      "id": post.id,
      "created_at": post.created_at,
      "active": post.active,
      "text": post.text,
      "author_id": post.author_id,
      "author": {
          "object": "account",
          "mode": "standard",
          "id": post.author.id,
          "created_at": post.author.created_at,
          "active": post.author.active,
          "username": post.author.username,
          "first_name": post.author.first_name,
          "last_name": post.author.last_name
      },
      "n_likes": post.n_likes
  } for post in posts])


@app.route("/like", methods=["POST"])
@auth.login_required
def like_post():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
                                      requester_id=auth.current_user().id)
  params = flask.request.get_json()
  try:
    post_id = params["post_id"]
  except KeyError:
    return ({}, 400)
  try:
    like = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=like_post rs=like rf=like_post rid=%s" %
        request_metadata.id)(app.rpc.like_post,
                             request_metadata=request_metadata,
                             post_id=post_id)
  except TLikeAlreadyExistsException:
    return ({}, 400)
  return {
      "object": "like",
      "mode": "standard",
      "id": like.id,
      "created_at": like.created_at,
      "account_id": like.account_id,
      "post_id": like.post_id
  }


@app.route("/like/<int:like_id>", methods=["GET"])
def retrieve_like(like_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  try:
    like = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=retrieve_like rs=like rf=retrieve_expanded_like rid=%s"
        % request_metadata.id)(app.rpc.retrieve_expanded_like,
                               request_metadata=request_metadata,
                               like_id=like_id)
  except TLikeNotFoundException:
    return ({}, 404)
  return {
      "object": "like",
      "mode": "expanded",
      "id": like.id,
      "created_at": like.created_at,
      "account_id": like.account_id,
      "post_id": like.post_id,
      "account": {
          "object": "account",
          "mode": "standard",
          "id": like.account.id,
          "created_at": like.account.created_at,
          "active": like.account.active,
          "username": like.account.username,
          "first_name": like.account.first_name,
          "last_name": like.account.last_name
      },
      "post": {
          "object": "post",
          "mode": "expanded",
          "id": like.post.id,
          "created_at": like.post.created_at,
          "active": like.post.active,
          "text": like.post.text,
          "author_id": like.post.author_id,
          "author": {
              "object": "account",
              "mode": "standard",
              "id": like.post.author.id,
              "created_at": like.post.author.created_at,
              "active": like.post.author.active,
              "username": like.post.author.username,
              "first_name": like.post.author.first_name,
              "last_name": like.post.author.last_name
          },
          "n_likes": like.post.n_likes
      }
  }


@app.route("/like/<int:like_id>", methods=["DELETE"])
@auth.login_required
def delete_like(like_id):
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
                                      requester_id=auth.current_user().id)
  try:
    RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=delete_like rs=like rf=delete_like rid=%s" %
        request_metadata.id)(app.rpc.delete_like,
                             request_metadata=request_metadata,
                             like_id=like_id)
  except TLikeNotAuthorizedException:
    return ({}, 403)
  except TLikeNotFoundException:
    return ({}, 404)
  return {}


@app.route("/like", methods=["GET"])
def list_likes():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  limit = int(flask.request.args["limit"]) \
      if "limit" in flask.request.args else 32
  offset = int(flask.request.args["offset"]) \
      if "offset" in flask.request.args else 0
  account_id = int(flask.request.args["account_id"]) \
      if "account_id" in flask.request.args else None
  post_id = int(flask.request.args["post_id"]) \
      if "post_id" in flask.request.args else None
  query = TLikeQuery(account_id=account_id, post_id=post_id)
  try:
    likes = RPC_WRAPPER(
        app.rpc_logger,
        "ls=apigateway lf=list_likes rs=like rf=list_likes rid=%s" %
        request_metadata.id)(app.rpc.list_likes,
                             request_metadata=request_metadata,
                             query=query,
                             limit=limit,
                             offset=offset)
  except TAccountNotFoundException:
    return ({}, 400)
  except TPostNotFoundException:
    return ({}, 400)
  return flask.jsonify([{
      "object": "like",
      "mode": "expanded",
      "id": like.id,
      "created_at": like.created_at,
      "account_id": like.account_id,
      "post_id": like.post_id,
      "account": {
          "object": "account",
          "mode": "standard",
          "id": like.account.id,
          "created_at": like.account.created_at,
          "active": like.account.active,
          "username": like.account.username,
          "first_name": like.account.first_name,
          "last_name": like.account.last_name
      },
      "post": {
          "object": "post",
          "mode": "expanded",
          "id": like.post.id,
          "created_at": like.post.created_at,
          "active": like.post.active,
          "text": like.post.text,
          "author_id": like.post.author_id,
          "author": {
              "object": "account",
              "mode": "standard",
              "id": like.post.author.id,
              "created_at": like.post.author.created_at,
              "active": like.post.author.active,
              "username": like.post.author.username,
              "first_name": like.post.author.first_name,
              "last_name": like.post.author.last_name
          },
          "n_likes": like.post.n_likes
      }
  } for like in likes])


@app.route("/trending", methods=["GET"])
def list_trending_hashtags():
  request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
  limit = int(flask.request.args["limit"]) \
      if "limit" in flask.request.args else 10
  return flask.jsonify(
      RPC_WRAPPER(
          app.rpc_logger,
          "ls=apigateway lf=list_trending_hashtags rs=trending rf=fetch_trending_hashtags rid=%s"
          % request_metadata.id)(app.rpc.fetch_trending_hashtags,
                                 request_metadata=request_metadata,
                                 limit=limit))
