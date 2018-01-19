#ifndef DANS02_DSTAGE_BASICDSTAGESCHEDULER_H
#define DANS02_DSTAGE_BASICDSTAGESCHEDULER_H

#include <memory>

#include "dstage/applicationrequest.h"
#include "dstage/jobid"
#include "dstage/priority.h"
#include "dstage/scheduler.h"
#include "util/status.h"

namespace dans {
class BasicDStageScheduler : public Scheduler {
 public:
  BasicDStageScheduler(vector<unique_ptr<Callback>> job_callbacks, );

  // Purge will remove all instances of the Job linked to job_id if possible.
  Status Purge(JobId job_id) override;

 protected:
  vector<unique_ptr<Callback>> _job_callbacks;
  ThreadPoop _primary_threadpool;
  ThreadPoop _secondary_threadpool;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_BASICDSTAGESCHEDULER_H