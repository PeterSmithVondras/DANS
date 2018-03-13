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

The Connect Handler accepts a UniqJobPtr<ConnectData>

  * Directly accessed by the Application or in this case the test SimpleClient.
  * Processes the Applications requests.
  * Creates a TCP connection for each duplicate file request utilizing the [linux_communication_handler](../common/dstage/linux_communication_handler.h).
  * Passes Jobs with active connections to a server to the [client_request_handler](../common/dstage/client_request_handler.h).
  * Purges Jobs within this DStage and passes Purge signal on the [client_connect_handler](../common/dstage/client_connect_handler.h).

### [client_request_handler](../common/dstage/client_request_handler.h)

### [client_response_handler](../common/dstage/client_response_handler.h)