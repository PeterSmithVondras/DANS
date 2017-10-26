#ifndef DANS02_DSTAGE_BASICDSTAGESCHEDULER_H
#define DANS02_DSTAGE_BASICDSTAGESCHEDULER_H

#include <memory>

#include "dstage/applicationrequest.h"
#include "dstage/scheduler.h"
#include "dstage/jobid"
#include "dstage/priority.h"
#include "util/status.h"

namespace duplicate_aware_scheduling {
class BasicDStageScheduler : public Scheduler {
public:
  // Forwards the Dispatch() to the linked Dispatcher. It is an error to
  // call this without a linked Dispatcher.
  Status Dispatch(unique_ptr<ApplicationRequest> app_req,
                  Priority incoming_priority,
                  uint duplication_level) override;

  // Purge will remove all instances of the Job linked to job_id if possible.
  Status Purge(JobId job_id) override;

protected:

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_BASICDSTAGESCHEDULER_H