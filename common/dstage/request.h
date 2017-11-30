#ifndef DANS02_DSTAGE_REQUEST_H
#define DANS02_DSTAGE_REQUEST_H

#include "dstage/requestdata.h"
#include "dstage/priority.h"

namespace duplicate_aware_scheduling {

typedef const int RequestId;

class RequestIdFactory {
 public:
  RequestIdFactory(int seed);
  RequestId CreateRequestId();
 private:
  int _next_id;
};

struct Request {
  Request(RequestData req_data,
          JobId job_id,
          std::vector<Destination> destinations,
          Priority priority);

  // Tests that two Requests have the same JobId.
  bool operator==(Symbol& rhs) const;

  RequestData req_data;
  RequestId request_id;
  Priority priority;
  uint duplication_level

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_REQUEST_H