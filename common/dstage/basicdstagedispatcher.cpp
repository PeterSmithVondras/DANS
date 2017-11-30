#include "common/dstage/basicdstagedispatcher.h"

namespace duplicate_aware_scheduling {
  BasicDStageDispatcher::BasicDStageDispatcher(int max_duplication_level)
   : _max_duplication_level(max_duplication_level) {

  }

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  bool BasicDStageDispatcher::Dispatch(unique_ptr<ApplicationRequest> app_req,
                                       uint duplication_level) override;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  bool BasicDStageDispatcher::Purge(JobId job_id) override;

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  unique_ptr<ApplicationRequest> BasicDStageDispatcher::GetNextJob() override;

  // Returns the next Primary priority job. This is a thread safe function and
  // will block indefinitely while no Job exists.
  unique_ptr<ApplicationRequest> BasicDStageDispatcher::GetNextPrimaryJob()
    override;

  // Returns the highest priority job. This is a thread safe function.
  // GetNextJob() will block indefinitely while no Job exists.
  unique_ptr<ApplicationRequest> BasicDStageDispatcher::GetNextSecondaryJob()
    override;

  vector<pcQueue<struct Job>> _priority_qs;
  Hash<JobId, struct JobMap> _job_mapper;
  const uint _max_duplication_level;

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_BASICDSTAGEDISPATCHER_H