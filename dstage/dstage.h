#ifndef DANS02_DSTAGE_DSTAGE_H
#define DANS02_DSTAGE_DSTAGE_H

#include <memory>

#include "dstage/applicationRequest.h"
#include "dstage/jobid"
#include "dstage/priority.h"
#include "util/status.h"


namespace duplicate_aware_scheduling {

class DStage {
public:
  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  virtual Status Dispatch(unique_ptr<ApplicationRequest> app_req,
                          Priority base_prio,
                          uint duplication_level);

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  virtual Status Purge(JobId job_id);
};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_DSTAGE_H