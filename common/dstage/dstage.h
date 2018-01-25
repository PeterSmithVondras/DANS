#ifndef DANS02_DSTAGE_DSTAGE_H
#define DANS02_DSTAGE_DSTAGE_H

#include <memory>

#include "common/dstage/basedispatcher.h"
#include "common/dstage/basedstage.h"
#include "common/dstage/basemultiqueue.h"
#include "common/dstage/basescheduler.h"
#include "common/dstage/job.h"

namespace dans {

template <typename T>
class DStage : public BaseDStage<T> {
 public:
  DStage(Priority max_priority, std::unique_ptr<BaseMultiQueue<T>> multi_q,
         std::unique_ptr<BaseDispatcher<T>> dispatcher,
         std::unique_ptr<BaseScheduler<T>> scheduler);
  ~DStage() {}

  // Introduces an ApplicationJob to a DStage. base_prio is the incoming
  // Priority of the ApplicationJob. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  void Dispatch(UniqConstJobPtr<T> job,
                unsigned requested_duplication) override;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  std::list<UniqConstJobPtr<T>> Purge(JobId job_id) override;

 protected:
  const Priority _max_priority;
  std::unique_ptr<BaseMultiQueue<T>> _multi_q;
  std::unique_ptr<BaseDispatcher<T>> _dispatcher;
  std::unique_ptr<BaseScheduler<T>> _scheduler;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_DSTAGE_H