#ifndef DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H
#define DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H

#include <memory>
#include <vector>
#include <list>
#include <unordered_map>

#include "dstage/request.h"
#include "dstage/basicdstagedispatcher.h"
#include "dstage/destination.h"
#include "dstage/jobid"
#include "dstage/priority.h"

namespace duplicate_aware_scheduling {
class BasicDStageDispatcher : public Dispatcher {
public:
  BasicDStageDispatcher(int max_duplication_level);
  ~BasicDStageDispatcher();

  // Introduces an Request to a DStage. base_prio is the incoming
  // Priority of the Request. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  bool Dispatch(unique_ptr<Request> app_req,
                          DestinationMap destination_map,
                          uint duplication_level) override;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  bool Purge(JobId job_id) override;

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  unique_ptr<Request> GetNextJob() override;

  // Returns the next Primary priority job. This is a thread safe function and
  // will block indefinitely while no Job exists.
  unique_ptr<Request> GetNextPrimaryJob() override;

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  unique_ptr<Request> GetNextSecondaryJob() override;

protected:
  struct JobMap;
  struct Job {
    unique_ptr<Request> app_req,
    JobMap* parent_job_map
  };

  struct JobMap {
    JobMap(unique_ptr<Vector<JobMap**>> instances_of_this_job);
    unique_ptr<Vector<Job*>> _instances_of_this_job;
  };

  vector<list<struct Job>> _priority_qs;
  std::unordered_map<JobId, struct JobMap> _job_mapper;
  const uint _max_duplication_level;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H