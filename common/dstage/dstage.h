#ifndef DANS02_DSTAGE_DSTAGE_H
#define DANS02_DSTAGE_DSTAGE_H

#include "common/dstage/job.h"

namespace duplicate_aware_scheduling {

template <typename T>
class DStage {
public:
  virtual ~DStage() {}

  // Introduces an ApplicationJob to a DStage. base_prio is the incoming
  // Priority of the ApplicationJob. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  virtual bool Dispatch(Job<T> job);

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  virtual bool Purge(JobId request_id);
};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_DSTAGE_H