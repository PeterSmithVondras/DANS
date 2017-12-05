#ifndef DANS02_DSTAGE_DISPATCHER_H
#define DANS02_DSTAGE_DISPATCHER_H

#include "dstage/request.h"
#include "dstage/destination.h"
#include "dstage/priority.h"

namespace duplicate_aware_scheduling {
template <class T>
class Dispatcher {
public:
  virtual Dispatcher() = 0;

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  bool Dispatch(unique_ptr<ApplicationRequest> app_req);

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher.
  bool PurgeQueues(RequestId request_id);

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_DISPATCHER_H