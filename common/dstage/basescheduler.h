#ifndef DANS02_DSTAGE_BASE_SCHEDULER_H
#define DANS02_DSTAGE_BASE_SCHEDULER_H

#include "common/dstage/basemultiqueue.h"
#include "common/dstage/job.h"

namespace dans {
template <typename T>
class BaseScheduler {
 public:
  virtual ~BaseScheduler(){};

  // Tells the Scheduler what multiqueue to pull from.
  virtual void LinkMultiQ(BaseMultiQueue<T>* multi_q_p) = 0;

  virtual void Run() = 0;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  virtual unsigned Purge(JobId job_id) = 0;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_BASE_SCHEDULER_H