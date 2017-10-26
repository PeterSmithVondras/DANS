#ifndef DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H
#define DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H

#include <memory>
#include <vector>

#include "dstage/applicationrequest.h"
#include "dstage/basicdstagedispatcher.h"
#include "dstage/destination.h"
#include "dstage/jobid"
#include "dstage/priority.h"
#include "dstage/pcqueue"
#include "util/status.h"

namespace duplicate_aware_scheduling {
class BasicDStageDispatcher : public Dispatcher {
public:
  struct JobMap;
  struct Job {
    bool purged,
    unique_ptr<ApplicationRequest> app_req,
    JobMap* parent_job_map
  };

  struct JobMap {
    JobMap(unique_ptr<Vector<JobMap**>> instances_of_this_job);
    unique_ptr<Vector<Job*>> _instances_of_this_job;
  };

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request and set job_id for referencing
  // this job in the future.
  Status Dispatch(unique_ptr<ApplicationRequest> app_req,
                  DestinationMap destination_map,
                  uint duplication_level) override;

  // Purge will remove all instances of the Job linked to job_id that
  // have not already been scheduled. Returns OK on success.
  Status Purge(JobId job_id) override;

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  unique_ptr<ApplicationRequest> GetNextJob() override;

  // Returns the next Primary priority job. This is a thread safe function and
  // will block indefinitely while no Job exists.
  unique_ptr<ApplicationRequest> GetNextPrimaryJob() override;

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  unique_ptr<ApplicationRequest> GetNextSecondaryJob() override;

protected:
  vector<pcQueue<struct Job>> _priority_qs;
  Hash<JobId, struct JobMap> _job_mapper;
  const uint _max_duplication_level;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H