#ifndef DANS02_DSTAGE_SCHEDULER_H
#define DANS02_DSTAGE_SCHEDULER_H

#include <shared_mutex>
#include <thread>
#include <vector>

#include "common/dstage/basemultiqueue.h"
#include "common/dstage/basescheduler.h"
#include "common/dstage/job.h"
#include "common/dstage/priority.h"
#include "glog/logging.h"

namespace dans {
template <typename T>
class Scheduler : public BaseScheduler<T> {
 public:
  Scheduler(std::vector<unsigned> threads_per_prio);

  ~Scheduler() override;

  // Tells the Scheduler what multiqueue to pull from.
  void LinkMultiQ(BaseMultiQueue<T>* multi_q_p) override;

  void Run() override;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked
  // DStages.
  std::list<UniqConstJobPtr<T>> Purge(JobId job_id) override;

 protected:
  bool _running;
  const Priority _max_priority;
  BaseMultiQueue<T>* _multi_q_p;

  // Guards the destruction state.
  std::shared_timed_mutex _destructing_lock;
  // Signals the destruction phase to all threads.
  bool _destructing;

  std::vector<unsigned> _threads_per_prio;
  std::vector<std::vector<std::thread>> _workers;

  void StartScheduling(Priority prio);
};
}  // namespace dans

#endif  // DANS02_DSTAGE_SCHEDULER_H