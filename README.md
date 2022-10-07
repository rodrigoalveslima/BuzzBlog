# BuzzBlog
BuzzBlog is an open-source microblog application for the performance study of
microservice-based systems. As with Twitter, users can write posts, follow other
users, and like posts.

## Architecture
### API Gateway
The API Gateway is an HTTP server that receives calls to the BuzzBlog API from
clients, makes RPCs to backend microservices, and replies JSON-encoded data back
to the clients. The [BuzzBlog API Reference](docs/API.md) shows the parameters
of HTTP requests and the attributes of HTTP responses for each API endpoint.

### Microservices
BuzzBlog functionalities are implemented by backend services with small scope,
conforming to the microservice architectural style.
- *account* implements functions to access and manage user accounts;
- *post* implements functions to access and manage posts;
- *like* implements functions to access and manage likes of posts;
- *follow* implements functions to access and manage follows of accounts;
- *uniquepair* implements functions to access and manage sets of unique pairs
(an abstract data type);
- *trending* implements functions to access and manage trending hashtags;
- *wordfilter* implements functions to filter out invalid words.

### Data Storage
BuzzBlog data is stored in PostgreSQL databases. Also, a Redis server stores the
data structure that ranks the popularity of hashtags.

## Request Lifecycle
BuzzBlog has an API organized around REST -- standard HTTP authentication and
methods control accesses and updates to objects through predictable URLs.

A call to the BuzzBlog API starts with the client sending an HTTP request to the
API Gateway. Then, the API Gateway accesses and updates objects through RPCs to
microservices. Microservices commonly query a Postgres database or Redis server
to retrieve and update data of their own domains. Some microservices call other
microservices to retrieve and update data of other domains. For example,
function `retrieve_expanded_account` of the *account* microservice queries a
PostgreSQL database to retrieve account information and calls microservices
*post*, *like*, and *follow* to retrieve the posting, liking, and following
activities of that account. Finally, the API Gateway sends an HTTP response back
to the client containing JSON-encoded data.

## Developer
- Rodrigo Alves Lima (ral@gatech.edu)

## License
Copyright (C) 2022 Georgia Tech Center for Experimental Research in Computer
Systems.

Licensed under the Apache License 2.0.