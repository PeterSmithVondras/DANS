# DANS Simple Client

The simple DANS Client can be seen in the [file_client_dstage_chain_test](../common/dstage/file_client_dstage_chain_test.cc). This test creates three unique DStages, the [Connect Handler](../common/dstage/client_connect_handler.h), [Request Handler](../common/dstage/client_request_handler.h) and the [Response Handler](../common/dstage/client_response_handler.h). The SimpleClient test then acts as an Application which requests files from the client facing (connect_handler) DStage, supplying it with a callback for the completed file.

## [Job Types](../common/dstage/job_types.h)

These are the types of Jobs (or UniqJobPtr's) that are used in the Client:
* ConnectData - Application request.
* ConnectDataInternal - Duplicated Application request which is understood within the Connect Handler.
* RequestData - Used for requesting a file once a connection has been made.
* ResponseData - Used for receiving a response from a server.

## DStages

### [Connect Handler](../common/dstage/client_connect_handler.h)

The Connect Handler accepts Jobs with type ConnectData from an Application. The resource that it duplicates is the file request itself. Additionally, it controls the creation of TCP connections, which is also a resource.

  * Directly accessed by the Application or in this case the test SimpleClient.
  * Processes the Applications requests.
  * Creates a TCP connection for each duplicate file request utilizing the [linux_communication_handler](../common/dstage/linux_communication_handler.h).
  * Passes Jobs with active connections to a server to the [Request Handler](../common/dstage/client_request_handler.h).
  * Purges Jobs within this DStage and passes Purge signal on the [Request Handler](../common/dstage/client_request_handler.h).

### [Request Handler](../common/dstage/client_request_handler.h)

The Request Handler accepts Jobs with type RequestData from the Connect Handler. The Request Handler makes the number of ongoing requests to servers duplicate aware. Its job is to send a packet requesting a file to a server using an open TCP connection. It would be simple to combine this stage with the Response Handler, but at least while developing the system, it seemed important not to assume that they would share the same host.

### [Response Handler](../common/dstage/client_response_handler.h)

The Response Handler is responsible for compiling a file that the server sends. It takes Jobs that are in multiple states, all which are described by the Job itself.
* Jobs that have not received an accept packet from the server, which are parsed and if there is an accept, a buffer is allocated for the file and a new Job with the same ID is sent to the Response Handler which is expecting a stream of bytes.
* Jobs that are expecting a stream of bytes will write the bytes to their buffer and then resubmit an updated read Job to the Response Handler.
* Jobs that receive the last bytes of a file will Purge the Job's duplicates by setting the PurgeState in the Job to "Purged" and by calling Purge() on the Connect Handler DStage which it maintains a pointer to.

## Current Issues

* Currently, when a Connection object goes out of scope, the socket is closed and therefore MultiQueues are able to close sockets using generic code. However, at the moment, when a Connection is being stored, along with a dynamically allocated callback, in an epoll event, it is stuck there until the socket it triggered. In the future, I hope to add a scheduler Register functionality which can artificially trigger a socket on command to allow for more efficient Purging.

## Future Work

* At the moment, a Connection object closes the socket that it manages when it goes out of scope. An ehancement would be to allow the recycleing of the TCP connection. This would be done by changing the destructor of the Connection object to return the socket to a connection pool.
* It would be good to create a couple generic Dispatchers. Two that would be very reusable would be:
  * One that does not duplicate, but just provides a transform function which converts from Job Input Type to Internet Type. If no transformation is required, that function can just be one that returns what it is sent.
  * A Dispatcher that provides a foldr functionality so that all that is required to make a duplicating Dispatcher is the provide the function that that is called repeatedly during the foldr.
* I could imagine DStages which had a Dispatcher which potentially utilized several more generic Dispatchers that were pre-built. This could be powerful in terms of code reuse.