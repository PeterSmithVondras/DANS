// An abstract class for any DStage data that is specific to that stage.
// Common subclasses include file numbers, ip addresses, sockets, total number
// of duplicates.

#ifndef DANS02_DSTAGE_REQUESTDATA_H
#define DANS02_DSTAGE_REQUESTDATA_H

#include <memory>

namespace duplicate_aware_scheduling {

typedef unsigned Priority;

class RequestData {
 public:
  virtual ~RequestData() {}

  virtual std::unique_ptr<RequestData> CreateDuplicate(Priority priority) = 0;
};

} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_REQUESTDATA_H