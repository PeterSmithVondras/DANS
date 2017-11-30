#ifndef DANS02_DSTAGE_DISPATCHER_H
#define DANS02_DSTAGE_DISPATCHER_H

#include "dstage/request.h"
#include "dstage/destination.h"
#include "dstage/priority.h"

namespace duplicate_aware_scheduling {
class Dispatcher {
public:
  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  virtual bool Dispatch(unique_ptr<ApplicationRequest> app_req);

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher.
  virtual bool Purge(RequestId request_id);

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  virtual unique_ptr<ApplicationRequest> GetNextJob();

  // Returns the next Primary priority job. This is a thread safe function and
  // will block indefinitely while no Job exists.
  virtual unique_ptr<ApplicationRequest> GetNextPrimaryJob();

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  virtual unique_ptr<ApplicationRequest> GetNextSecondaryJob();

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_DISPATCHER_H