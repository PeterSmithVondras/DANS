#ifndef DANS02_DSTAGE_SCHEDULER_H
#define DANS02_DSTAGE_SCHEDULER_H

#include "dstage/dispatcher.h"
#include "dstage/dstage.h"
#include "util/status.h"

namespace duplicate_aware_scheduling {
class Scheduler {
 public:
  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  Status Purge(JobId job_id);

  // Points the Scheduler to a specific Dispatcher which may now request work.
  Status LinkToDispatcher(Dispatcher* dispatcher);

 protected:
  Dispatcher* _dispatcher;
};
}  // namespace duplicate_aware_scheduling

#endif  // DANS02_DSTAGE_SCHEDULERCHER_H