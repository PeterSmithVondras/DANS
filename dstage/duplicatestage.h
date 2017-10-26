#ifndef DANS02_DSTAGE_DUPLICATESTAGE_H
#define DANS02_DSTAGE_DUPLICATESTAGE_H

#include <memory>
#include <vector>

#include "dstage/applicationrequest.h"
#include "dstage/destination.h"
#include "dstage/dispatcher.h"
#include "dstage/dstage.h"
#include "dstage/jobid"
#include "dstage/jobmap"
#include "dstage/priority.h"
#include "dstage/pcqueue.h"
#include "dstage/scheduler.h"
#include "util/status.h"

namespace duplicate_aware_scheduling {
class DuplicateStage : public DStage {
public:
  DuplicateStage(unique_ptr<Dispatcher> _dispatcher,
                 unique_ptr<Scheduler> _scheduler);

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  Status Dispatch(unique_ptr<ApplicationRequest> app_req,
                  DestinationMap destination_map,
                  uint duplication_level) override;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  Status Purge(JobId job_id) override;

private:
  unique_ptr<Dispatcher> _dispatcher;
  unique_ptr<Scheduler> _scheduler;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_DUPLICATESTAGE_H