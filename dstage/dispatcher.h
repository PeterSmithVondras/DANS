#ifndef DANS02_DSTAGE_DISPATCHER_H
#define DANS02_DSTAGE_DISPATCHER_H

#include "dstage/applicationrequest.h"
#include "dstage/dstage.h"

namespace duplicate_aware_scheduling {
class Dispatcher : DStage {
public:
  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  virtual unique_ptr<ApplicationRequest> GetNextJob();

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_DISPATCHER_H