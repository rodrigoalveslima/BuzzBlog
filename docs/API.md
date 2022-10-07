# BuzzBlog API Reference
## Create an account
* **Endpoint**: `POST /account`
* **Parameters**:
  - `username`
  - `password`
  - `first_name`
  - `last_name`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The account object (standard mode), if the operation succeeded.
```
{
  "object": "account",
  "mode": "standard",
  "id": 12345,
  "created_at": 1601912233,
  "active": true,
  "username": "john.doe",
  "first_name": "John",
  "last_name": "Doe",
  "followed_by_you": false
}
```

## Retrieve an account
* **Endpoint**: `GET /account/:account_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `401`: (Unauthorized) No valid username/password pair provided
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The account object (expanded mode), if a valid identifier was
               provided.
```
{
  "object": "account",
  "mode": "expanded",
  "id": 12345,
  "created_at": 1601912233,
  "active": true,
  "username": "john.doe",
  "first_name": "John",
  "last_name": "Doe",
  "followed_by_you": false,
  "follows_you": false,
  "n_followers": 0,
  "n_following": 0,
  "n_posts": 0,
  "n_likes": 0
}
```

## Update an account
* **Endpoint**: `PUT /account/:account_id`
* **Parameters**:
  - `password`
  - `first_name`
  - `last_name`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `401`: (Unauthorized) No valid username/password pair provided
  - `403`: (Forbidden) The user does not have permissions to perform the request
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The account object (standard mode), if the operation succeeded.
```
{
  "object": "account",
  "mode": "standard",
  "id": 12345,
  "created_at": 1601912233,
  "active": true,
  "username": "john.doe",
  "first_name": "John",
  "last_name": "Doe",
  "followed_by_you": false
}
```

## Delete an account (deactivate)
* **Endpoint**: `DELETE /account/:account_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `401`: (Unauthorized) No valid username/password pair provided
  - `403`: (Forbidden) The user does not have permissions to perform the request
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end

## List accounts
* **Endpoint**: `GET /account`
* **Parameters**:
  - `limit`
  - `offset`
* **Filters**
  - `username`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `401`: (Unauthorized) No valid username/password pair provided
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: A list of account objects (expanded mode) in reverse chronological
               order.
```
[
  {
    "object": "account",
    "mode": "expanded",
    "id": 12345,
    "created_at": 1601912233,
    "active": true,
    "username": "john.doe",
    "first_name": "John",
    "last_name": "Doe",
    "followed_by_you": false,
    "follows_you": false,
    "n_followers": 0,
    "n_following": 0,
    "n_posts": 0,
    "n_likes": 0
  }
]
```

## Follow an account
* **Endpoint**: `POST /follow`
* **Parameters**:
  - `account_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `401`: (Unauthorized) No valid username/password pair provided
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The follow object (standard mode), if the operation succeeded.
```
{
  "object": "follow",
  "mode": "standard",
  "id": 123,
  "created_at": 1601912233,
  "follower_id": 12345,
  "followee_id": 54321
}
```

## Retrieve a follow
* **Endpoint**: `GET /follow/:follow_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `401`: (Unauthorized) No valid username/password pair provided
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The follow object (expanded mode), if a valid identifier was
               provided.
```
{
  "object": "follow",
  "mode": "expanded",
  "id": 123,
  "created_at": 1601912233,
  "follower_id": 12345,
  "followee_id": 54321,
  "follower": {
    "object": "account",
    "mode": "standard",
    "id": 12345,
    "created_at": 1601912233,
    "active": true,
    "username": "john.doe",
    "first_name": "John",
    "last_name": "Doe",
    "followed_by_you": false
  },
  "followee": {
    "object": "account",
    "mode": "standard",
    "id": 54321,
    "created_at": 1601912233,
    "active": true,
    "username": "jane.roe",
    "first_name": "Jane",
    "last_name": "Roe",
    "followed_by_you": false
  }
}
```

## Delete a follow
* **Endpoint**: `DELETE /follow/:follow_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `401`: (Unauthorized) No valid username/password pair provided
  - `403`: (Forbidden) The user does not have permissions to perform the request
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end

## List follows
* **Endpoint**: `GET /follow`
* **Parameters**:
  - `limit`
  - `offset`
* **Filters**
  - `follower_id`
  - `followee_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `401`: (Unauthorized) No valid username/password pair provided
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: A list of follow objects (expanded mode) in reverse chronological
               order.
```
[
  {
    "object": "follow",
    "mode": "expanded",
    "id": 123,
    "created_at": 1601912233,
    "follower_id": 12345,
    "followee_id": 54321,
    "follower": {
      "object": "account",
      "mode": "standard",
      "id": 12345,
      "created_at": 1601912233,
      "active": true,
      "username": "john.doe",
      "first_name": "John",
      "last_name": "Doe",
      "followed_by_you": false
    },
    "followee": {
      "object": "account",
      "mode": "standard",
      "id": 54321,
      "created_at": 1601912233,
      "active": true,
      "username": "jane.roe",
      "first_name": "Jane",
      "last_name": "Roe",
      "followed_by_you": false
    }
  }
]
```

## Create a post
* **Endpoint**: `POST /post`
* **Parameters**:
  - `text`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The post object (standard mode), if the operation succeeded.
```
{
  "object": "post",
  "mode": "standard",
  "id": 123,
  "created_at": 1601912233,
  "active": true,
  "text": "Hello, world.",
  "author_id": 12345
}
```

## Retrieve a post
* **Endpoint**: `GET /post/:post_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `401`: (Unauthorized) No valid username/password pair provided
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The post object (expanded mode), if a valid identifier was
               provided.
```
{
  "object": "post",
  "mode": "expanded",
  "id": 123,
  "created_at": 1601912233,
  "active": true,
  "text": "Hello, world.",
  "author_id": 12345,
  "author": {
    "object": "account",
    "mode": "standard",
    "id": 12345,
    "created_at": 1601912233,
    "active": true,
    "username": "john.doe",
    "first_name": "John",
    "last_name": "Doe",
    "followed_by_you": false
  },
  "n_likes": 0
}
```

## Delete a post (deactivate)
* **Endpoint**: `DELETE /post/:post_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `401`: (Unauthorized) No valid username/password pair provided
  - `403`: (Forbidden) The user does not have permissions to perform the request
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end

## List posts
* **Endpoint**: `GET /post`
* **Parameters**:
  - `limit`
  - `offset`
* **Filters**
  - `author_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `401`: (Unauthorized) No valid username/password pair provided
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: A list of post objects (expanded mode) in reverse chronological
               order.
```
[
  {
    "object": "post",
    "mode": "expanded",
    "id": 123,
    "created_at": 1601912233,
    "active": true,
    "text": "Hello, world.",
    "author_id": 12345,
    "author": {
      "object": "account",
      "mode": "standard",
      "id": 12345,
      "created_at": 1601912233,
      "active": true,
      "username": "john.doe",
      "first_name": "John",
      "last_name": "Doe",
      "followed_by_you": false
    },
    "n_likes": 0
  }
]
```

## Like a post
* **Endpoint**: `POST /like`
* **Parameters**:
  - `post_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `401`: (Unauthorized) No valid username/password pair provided
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The like object (standard mode), if the operation succeeded.
```
{
  "object": "like",
  "mode": "standard",
  "id": 123,
  "created_at": 1601912233,
  "account_id": 12345,
  "post_id": 123
}
```

## Retrieve a like
* **Endpoint**: `GET /like/:like_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `401`: (Unauthorized) No valid username/password pair provided
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: The like object (expanded mode), if a valid identifier was
               provided.
```
{
  "object": "like",
  "mode": "expanded",
  "id": 123,
  "created_at": 1601912233,
  "account_id": 12345,
  "post_id": 123,
  "account": {
    "object": "account",
    "mode": "standard",
    "id": 12345,
    "created_at": 1601912233,
    "active": true,
    "username": "john.doe",
    "first_name": "John",
    "last_name": "Doe",
    "followed_by_you": false
  },
  "post": {
    "object": "post",
    "mode": "expanded",
    "id": 123,
    "created_at": 1601912233,
    "active": true,
    "text": "Hello, world.",
    "author_id": 12345,
    "author": {
      "object": "account",
      "mode": "standard",
      "id": 12345,
      "created_at": 1601912233,
      "active": true,
      "username": "john.doe",
      "first_name": "John",
      "last_name": "Doe",
      "followed_by_you": false
    },
    "n_likes": 1
  }
}
```

## Delete a like
* **Endpoint**: `DELETE /like/:like_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `401`: (Unauthorized) No valid username/password pair provided
  - `403`: (Forbidden) The user does not have permissions to perform the request
  - `404`: (Not Found) The requested resource does not exist
  - `500`: (Internal Server Error) Something went wrong on server's end

## List likes
* **Endpoint**: `GET /like`
* **Parameters**:
  - `limit`
  - `offset`
* **Filters**
  - `account_id`
  - `post_id`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `400`: (Bad Request) Missing or invalid parameter
  - `401`: (Unauthorized) No valid username/password pair provided
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: A list of like objects (expanded mode) in reverse chronological
               order.
```
[
  {
    "object": "like",
    "mode": "expanded",
    "id": 123,
    "created_at": 1601912233,
    "account_id": 12345,
    "post_id": 123,
    "account": {
      "object": "account",
      "mode": "standard",
      "id": 12345,
      "created_at": 1601912233,
      "active": true,
      "username": "john.doe",
      "first_name": "John",
      "last_name": "Doe",
      "followed_by_you": false
    },
    "post": {
      "object": "post",
      "mode": "expanded",
      "id": 123,
      "created_at": 1601912233,
      "active": true,
      "text": "Hello, world.",
      "author_id": 12345,
      "author": {
        "object": "account",
        "mode": "standard",
        "id": 12345,
        "created_at": 1601912233,
        "active": true,
        "username": "john.doe",
        "first_name": "John",
        "last_name": "Doe",
        "followed_by_you": false
      },
      "n_likes": 1
    }
  }
]
```

## List trending hashtags
* **Endpoint**: `GET /trending`
* **Parameters**:
  - `limit`
* **HTTP Response Codes**:
  - `200`: (Ok) Everything worked as expected
  - `500`: (Internal Server Error) Something went wrong on server's end
* **Returns**: A list of trending hashtags.
```
["foo", ..., "bar"]
```
