#ifndef DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H
#define DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H

#include <memory>
#include <vector>

#include "dstage/applicationrequest.h"
#include "dstage/basicdstagedispatcher.h"
#include "dstage/jobid"
#include "dstage/jobmap"
#include "dstage/priority.h"
#include "dstage/pcqueue"
#include "util/status.h"

namespace duplicate_aware_scheduling {
class BasicDStageDispatcher : public Dispatcher {
public:
  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request and set job_id for referencing
  // this job in the future.
  Status Dispatch(unique_ptr<ApplicationRequest> app_req,
                  Priority incoming_priority,
                  uint duplication_level) override;

  // Purge will remove all instances of the Job linked to job_id that
  // have not already been scheduled. Returns OK on success.
  Status Purge(JobId job_id) override;

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  unique_ptr<ApplicationRequest> GetNextJob() override;

protected:
  vector<pcQueue<JobMap*>> _priority_qs;
  Hash<JobId, JobMap> _job_mapper;
  const uint _max_duplication_level;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H