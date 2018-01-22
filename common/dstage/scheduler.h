#ifndef DANS02_DSTAGE_SCHEDULER_H
#define DANS02_DSTAGE_SCHEDULER_H

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/priority.h"

namespace dans {
template <typename T>
class Scheduler {
 public:
  Scheduler(Priority max_priority);

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  std::list<UniqConstJobPtr<T>> Purge(JobId job_id);

  void LinkMultiQ(MultiQueue<T>* multi_q_p);

 protected:
  bool _running;
  const Priority _max_priority;
  MultiQueue<T>* _multi_q_p;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_SCHEDULERCHER_H