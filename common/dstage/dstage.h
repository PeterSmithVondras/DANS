#ifndef DANS02_DSTAGE_DSTAGE_H
#define DANS02_DSTAGE_DSTAGE_H

#include <list>

#include "common/dstage/job.h"

namespace dans {

template <typename T>
class DStage {
 public:
  virtual ~DStage() {}

  // Introduces an ApplicationJob to a DStage. base_prio is the incoming
  // Priority of the ApplicationJob. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  virtual void Dispatch(UniqConstJobPtr<T> job,
                        unsigned requested_duplication) = 0;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  virtual std::list<UniqConstJobPtr<T>> Purge(JobId job_id) = 0;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_DSTAGE_H