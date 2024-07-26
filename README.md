# Tema 2 - Distributed Database

* The statement can be found at: https://ocw.cs.pub.ro/courses/sd-ca/teme/tema2-2024


## Introduction
* The program is meant to emulate a load balancer with multiple servers, with their own
databases and caches (based on the LRU algorithm).

* It can receive 4 types of request, which will be sent to the load balancer or to one
of the servers depending on the case. The server is selected using two hashes functions,
one for its id and another one for the name of the documents. The documents and the 
servers will be placed on a hashring, which will help get the right server for
the given document.

* The load balancer structure will contain the 2 hashes function mentioned
previously, a sorted server array (by their hashed ids) and the size of the array.
The sorted array makes it much easier to implement the consistent hashing which
we need.

* The LRU cache will store the name, the content and the most recently used
documents. To implement the cache, a hashtable and a doubly linked list have been
used, to store them efficiently.

## Available requests
* EDIT_DOCUMENT
    Creates a new document with the given name and content or edits the content of an
already existing file. It's not executed right away, the request being placed in
the server's queue which will be emptied when a GET_DOCUMENT request is sent to that
server.
* GET_DOCUMENT
    Searches for a document in the cache and, if not found there, in the database and
returns the document's content as response or (null) if the document doesn't exist
in the database.
* ADD_SERVER
    Will add a new server to our load_balancer and will place it in the sorted server
array. It gets placed on the hashring and the documents from the next server that
have a lower hash than the new one will be moved.
* REMOVE_SERVER
    Will remove one of the already existing servers and will transfer its documents
to the next server on our hashring.

NOTE: Both of the server requests will empty and run the requests from the queue of
the server that will be deleted/ will have some of its files moved.

### Potential responses/logs
#### Logs

* LOG_MISS
    The document hasn't been found in the server's cache.
* LOG_EVICT
    The document hasn't been found in the server's cache and another file was evicted
from the cache to bring in the recently accessed document.
* LOG_HIT
    The document has been found in the server's cache.
* LOG_FAULT
    The document hasn't been found in the server's database.
* LOG_LAZY_EXEC
    Prints the queue size.


#### RESPONSES
* MSG_A
    A request has been added to the queue.
* MSG_B
    A document has been overridden.
* MSG_C
    A new document has been created.
* content
    The content of the given document or (null) if it doesn't exist.
