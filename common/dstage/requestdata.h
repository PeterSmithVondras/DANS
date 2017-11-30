// An abstract class for any DStage data that is specific to that stage.
// Common subclasses include file numbers, ip addresses, sockets, total number
// of duplicates.

#ifndef DANS02_DSTAGE_REQUESTDATA_H
#define DANS02_DSTAGE_REQUESTDATA_H

namespace duplicate_aware_scheduling {

class RequestData {
  virtual RequestData() = 0;
};

} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_REQUESTDATA_H