# BuzzBlog
BuzzBlog is an open-source microblog application for the performance study of
microservice-based systems. As with Twitter, users can write posts, follow other
users, and like posts.

## Architecture
### API Gateway
The API Gateway sits between the clients that send HTTP requests and the
microservices needed to fulfill these requests. It receives HTTP requests
representing calls to the BuzzBlog API, makes RPCs to microservices, and replies
HTTP responses containing JSON-encoded data.

As a reference, the [BuzzBlog API Reference](docs/API.md) shows how to build
HTTP requests representing calls to the BuzzBlog API and their expected
responses.

### Microservices
BuzzBlog functionalities are implemented by backend services with small scope,
conforming to the microservice architectural style.
- *account* implements functions to access and manage user accounts;
- *post* implements functions to access and manage posts;
- *like* implements functions to access and manage post likes;
- *follow* implements functions to access and manage account follows;
- *uniquepair* implements functions to access and manage sets of unique pairs
(an abstract data type);
- *trending* implements functions to access and manage trending hashtags;
- *wordfilter* implements functions to filter out invalid words.

### Data Storage
PostgreSQL databases store BuzzBlog data (accounts, posts, likes, follows).
A Redis server stores the data structure that ranks the popularity of hashtags.

## Request Lifecycle
BuzzBlog has an API organized around REST -- standard HTTP authentication and
methods are used to access and manage objects through predictable URLs.

A call to the BuzzBlog API starts when the client sends an HTTP request to the
API Gateway, which fulfills it by accessing and managing objects through RPCs to
microservices. Microservice functions commonly query a Postgres database or
Redis server to retrieve and update data of that microservice domain. Some
microservice functions call other microservice functions to retrieve and update
data of different domains. For example, `retrieve_expanded_account` queries
a PostgreSQL database to retrieve account information and calls functions of
microservices *post*, *like*, and *follow* to retrieve a summary of the
posting, liking, and following activities of that account, respectively.
Finally, the API Gateway replies an HTTP response containing JSON-encoded data.

## Developer
- Rodrigo Alves Lima (ral@gatech.edu)

## License
Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
Systems.

Licensed under the Apache License 2.0.
