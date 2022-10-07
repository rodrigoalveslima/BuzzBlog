// Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
// Systems

namespace cpp gen
namespace py gen

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct TRequestMetadata {
  1: required string id;          // unique request id.
  2: optional i32 requester_id;   // id of the account making the request.
}

struct TAccount {
  // Standard
  1: required i32 id;
  2: required i32 created_at;
  3: required bool active;
  4: required string username;
  5: required string first_name;
  6: required string last_name;
  7: required bool followed_by_you;

  // Expanded
  8: optional bool follows_you;
  9: optional i32 n_followers;
  10: optional i32 n_following;
  11: optional i32 n_posts;
  12: optional i32 n_likes;
}

struct TAccountQuery {
  1: optional string username;
}

struct TFollow {
  // Standard
  1: required i32 id;
  2: required i32 created_at;
  3: required i32 follower_id;
  4: required i32 followee_id;

  // Expanded
  5: optional TAccount follower;
  6: optional TAccount followee;
}

struct TFollowQuery {
  1: optional i32 follower_id;
  2: optional i32 followee_id;
}

struct TPost {
  // Standard
  1: required i32 id;
  2: required i32 created_at;
  3: required bool active;
  4: required string text;
  5: required i32 author_id;

  // Expanded
  6: optional TAccount author;
  7: optional i32 n_likes;
}

struct TPostQuery {
  1: optional i32 author_id;
}

struct TLike {
  // Standard
  1: required i32 id;
  2: required i32 created_at;
  3: required i32 account_id;
  4: required i32 post_id;

  // Expanded
  5: optional TAccount account;
  6: optional TPost post;
}

struct TLikeQuery {
  1: optional i32 account_id;
  2: optional i32 post_id;
}

struct TUniquepair {
  1: required i32 id;
  2: required i32 created_at;
  3: required string domain;
  4: required i32 first_elem;
  5: required i32 second_elem;
}

struct TUniquepairQuery {
  1: required string domain;
  2: optional i32 first_elem;
  3: optional i32 second_elem;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Exceptions
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

exception TAccountInvalidCredentialsException {
}

exception TAccountDeactivatedException {
}

exception TAccountInvalidAttributesException {
}

exception TAccountUsernameAlreadyExistsException {
}

exception TAccountNotFoundException {
}

exception TAccountNotAuthorizedException {
}

exception TFollowInvalidAttributesException {
}

exception TFollowAlreadyExistsException {
}

exception TFollowNotFoundException {
}

exception TFollowNotAuthorizedException {
}

exception TLikeAlreadyExistsException {
}

exception TLikeNotFoundException {
}

exception TLikeNotAuthorizedException {
}

exception TPostInvalidAttributesException {
}

exception TPostNotFoundException {
}

exception TPostNotAuthorizedException {
}

exception TUniquepairNotFoundException {
}

exception TUniquepairAlreadyExistsException {
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Services
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

service TAccountService {
  /* Params:
   *   1. request_metadata: request metadata.
   *   2. username: username to be verified.
   *   3. password: password to be verified.
   * Returns:
   *   An active account (standard mode) matching the provided credentials.
   */
  TAccount authenticate_user (1:TRequestMetadata request_metadata,
      2:string username, 3:string password)
      throws (1:TAccountInvalidCredentialsException e1,
              2:TAccountDeactivatedException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. username: username of the account to be created.
   *   3. password: password of the account to be created.
   *   4. first_name: first name of the account to be created.
   *   5. last_name: last name of the account to be created.
   * Returns:
   *   The newly created account (standard mode).
   */
  TAccount create_account (1:TRequestMetadata request_metadata,
      2:string username, 3:string password, 4:string first_name,
      5:string last_name)
      throws (1:TAccountInvalidAttributesException e1,
              2:TAccountUsernameAlreadyExistsException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. account_id: id of the account to be retrieved.
   * Returns:
   *   The account (standard mode) matching the provided id.
   */
  TAccount retrieve_standard_account (1:TRequestMetadata request_metadata,
      2:i32 account_id)
      throws (1:TAccountNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. account_id: id of the account to be retrieved.
   * Returns:
   *   The account (expanded mode) matching the provided id.
   */
  TAccount retrieve_expanded_account (1:TRequestMetadata request_metadata,
      2:i32 account_id)
      throws (1:TAccountNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. account_id: id of the account to be updated.
   *   3. password: new password of the account to be updated.
   *   4. first_name: new first name of the account to be updated.
   *   5. last_name: new last name of the account to be updated.
   * Returns:
   *   The updated account (standard mode).
   */
  TAccount update_account (1:TRequestMetadata request_metadata,
      2:i32 account_id, 3:string password, 4:string first_name,
      5:string last_name)
      throws (1:TAccountNotAuthorizedException e1,
              2:TAccountInvalidAttributesException e2,
              3:TAccountNotFoundException e3);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. account_id: id of the account to be deleted.
   */
  void delete_account (1:TRequestMetadata request_metadata, 2:i32 account_id)
      throws (1:TAccountNotAuthorizedException e1,
              2:TAccountNotFoundException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. query: query parameters to fetch results.
   *   3. limit: max number of results to be fetched.
   *   4. offset: index to start fetching results.
   * Returns:
   *   A list of accounts (expanded mode) in reverse chronological order.
   */
  list<TAccount> list_accounts (1:TRequestMetadata request_metadata,
      2:TAccountQuery query, 3:i32 limit, 4:i32 offset);
}

service TFollowService {
  /* Params:
   *   1. request_metadata: request metadata.
   *   2. account_id: id of the account to be followed.
   * Returns:
   *   The newly created follow (standard mode).
   */
  TFollow follow_account (1:TRequestMetadata request_metadata, 2:i32 account_id)
      throws (1:TFollowAlreadyExistsException e1,
              2:TFollowInvalidAttributesException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. follow_id: id of the follow to be retrieved.
   * Returns:
   *   The follow (standard mode) matching the provided id.
   */
  TFollow retrieve_standard_follow (1:TRequestMetadata request_metadata,
      2:i32 follow_id)
      throws (1:TFollowNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. follow_id: id of the follow to be retrieved.
   * Returns:
   *   The follow (expanded mode) matching the provided id.
   */
  TFollow retrieve_expanded_follow (1:TRequestMetadata request_metadata,
      2:i32 follow_id)
      throws (1:TFollowNotFoundException e1,
              2:TAccountNotFoundException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. follow_id: id of the follow to be deleted.
   */
  void delete_follow (1:TRequestMetadata request_metadata, 2:i32 follow_id)
      throws (1:TFollowNotFoundException e1,
              2:TFollowNotAuthorizedException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. query: query parameters to fetch results.
   *   3. limit: max number of results to be fetched.
   *   4. offset: index to start fetching results.
   * Returns:
   *   A list of follows (expanded mode) in reverse chronological order.
   */
  list<TFollow> list_follows (1:TRequestMetadata request_metadata,
      2:TFollowQuery query, 3:i32 limit, 4:i32 offset)
      throws (1:TAccountNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. follower_id: id of the account to be checked as follower.
   *   3. followee_id: id of the account to be checked as followee.
   * Returns:
   *   True if follower follows followee. False, otherwise.
   */
  bool check_follow (1:TRequestMetadata request_metadata, 2:i32 follower_id,
      3:i32 followee_id);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. account_id: id of the account whose followers are counted.
   * Returns:
   *   The number of followers of the provided account.
   */
  i32 count_followers (1:TRequestMetadata request_metadata, 2:i32 account_id);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. account_id: id of the account whose followees are counted.
   * Returns:
   *   The number of followees of the provided account.
   */
  i32 count_followees (1:TRequestMetadata request_metadata, 2:i32 account_id);
}

service TLikeService {
  /* Params:
   *   1. request_metadata: request metadata.
   *   2. post_id: id of the post to be liked.
   * Returns:
   *   The newly created like (standard mode).
   */
  TLike like_post (1:TRequestMetadata request_metadata, 2:i32 post_id)
      throws (1:TLikeAlreadyExistsException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. like_id: id of the like to be retrieved.
   * Returns:
   *   The like (standard mode) matching the provided id.
   */
  TLike retrieve_standard_like (1:TRequestMetadata request_metadata,
      2:i32 like_id)
      throws (1:TLikeNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. like_id: id of the like to be retrieved.
   * Returns:
   *   The like (expanded mode) matching the provided id.
   */
  TLike retrieve_expanded_like (1:TRequestMetadata request_metadata,
      2:i32 like_id)
      throws (1:TLikeNotFoundException e1,
              2:TAccountNotFoundException e2,
              3:TPostNotFoundException e3);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. like_id: id of the like to be deleted.
   */
  void delete_like (1:TRequestMetadata request_metadata, 2:i32 like_id)
      throws (1:TLikeNotFoundException e1,
              2:TLikeNotAuthorizedException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. query: query parameters to fetch results.
   *   3. limit: max number of results to be fetched.
   *   4. offset: index to start fetching results.
   * Returns:
   *   A list of likes (expanded mode) in reverse chronological order.
   */
  list<TLike> list_likes (1:TRequestMetadata request_metadata,
      2:TLikeQuery query, 3:i32 limit, 4:i32 offset)
      throws (1:TAccountNotFoundException e1,
              2:TPostNotFoundException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. account_id: id of the account whose likes are counted.
   * Returns:
   *   The number of likes by the provided account.
   */
  i32 count_likes_by_account (1:TRequestMetadata request_metadata,
      2:i32 account_id);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. post_id: id of the post whose likes are counted.
   * Returns:
   *   The number of likes of the provided post.
   */
  i32 count_likes_of_post (1:TRequestMetadata request_metadata, 2:i32 post_id);
}

service TPostService {
  /* Params:
   *   1. request_metadata: request metadata.
   *   2. text: text of the post to be created.
   * Returns:
   *   The newly created post (standard mode).
   */
  TPost create_post (1:TRequestMetadata request_metadata, 2:string text)
      throws (1:TPostInvalidAttributesException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. post_id: id of the post to be retrieved.
   * Returns:
   *   The post (standard mode) matching the provided id.
   */
  TPost retrieve_standard_post (1:TRequestMetadata request_metadata,
      2:i32 post_id)
      throws (1:TPostNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. post_id: id of the post to be retrieved.
   * Returns:
   *   The post (expanded mode) matching the provided id.
   */
  TPost retrieve_expanded_post (1:TRequestMetadata request_metadata,
      2:i32 post_id)
      throws (1:TPostNotFoundException e1,
              2:TAccountNotFoundException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. post_id: id of the post to be deleted.
   */
  void delete_post (1:TRequestMetadata request_metadata, 2:i32 post_id)
      throws (1:TPostNotFoundException e1,
              2:TPostNotAuthorizedException e2);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. query: query parameters to fetch results.
   *   3. limit: max number of results to be fetched.
   *   4. offset: index to start fetching results.
   * Returns:
   *   A list of posts (expanded mode) in reverse chronological order.
   */
  list<TPost> list_posts (1:TRequestMetadata request_metadata,
      2:TPostQuery query, 3:i32 limit, 4:i32 offset)
      throws (1:TAccountNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. author_id: id of the author account whose posts are counted.
   * Returns:
   *   The number of posts written by the provided author account.
   */
  i32 count_posts_by_author (1:TRequestMetadata request_metadata,
      2:i32 author_id);
}

service TUniquepairService {
  /* Params:
   *   1. request_metadata: request metadata.
   *   2. uniquepair_id: id of the unique pair to be retrieved.
   * Returns:
   *   The unique pair matching the provided id.
   */
  TUniquepair get (1:TRequestMetadata request_metadata, 2:i32 uniquepair_id)
      throws (1:TUniquepairNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. domain: domain of the unique pair to be added.
   *   3. first_elem: first element of the unique pair to be added.
   *   4. second_elem: second element of the unique pair to be added.
   * Returns:
   *   The newly created unique pair.
   */
  TUniquepair add (1:TRequestMetadata request_metadata, 2:string domain,
      3:i32 first_elem, 4:i32 second_elem)
      throws (1:TUniquepairAlreadyExistsException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. uniquepair_id: id of the unique pair to be removed.
   */
  void remove (1:TRequestMetadata request_metadata, 2:i32 uniquepair_id)
      throws (1:TUniquepairNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. domain: domain of the unique pair to be found.
   *   3. first_elem: first element of the unique pair to be found.
   *   4. second_elem: second element of the unique pair to be found.
   * Returns:
   *   True if the unique pair is found. False, otherwise.
   */
  bool find (1:TRequestMetadata request_metadata, 2:string domain,
      3:i32 first_elem, 4:i32 second_elem)
      throws (1:TUniquepairNotFoundException e);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. query: query parameters to fetch results.
   *   3. limit: max number of results to be fetched.
   *   4. offset: index to start fetching results.
   * Returns:
   *   A list of unique pairs in reverse chronological order.
   */
  list<TUniquepair> fetch (1:TRequestMetadata request_metadata,
      2:TUniquepairQuery query, 3:i32 limit, 4:i32 offset);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. query: query parameters to count results.
   * Returns:
   *   The number of unique pairs.
   */
  i32 count (1:TRequestMetadata request_metadata, 2:TUniquepairQuery query);
}

service TTrendingService {
  /* Params:
   *   1. request_metadata: request metadata.
   *   2. text: text of the post to be processed.
   */
  void process_post (1:TRequestMetadata request_metadata, 2:string text);

  /* Params:
   *   1. request_metadata: request metadata.
   *   2. limit: max number of hashtags to be fetched.
   * Returns:
   *   A list of trending hashtags.
   */
  list<string> fetch_trending_hashtags (1:TRequestMetadata request_metadata,
      2:i32 limit);
}

service TWordfilterService {
  /* Params:
   *   1. request_metadata: request metadata.
   *   2. word: word to be validated.
   * Returns:
   *   True if the word is valid. False, otherwise.
   */
  bool is_valid_word (1:TRequestMetadata request_metadata, 2:string word);
}
