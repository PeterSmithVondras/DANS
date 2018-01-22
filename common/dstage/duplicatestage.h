#ifndef DANS02_DSTAGE_DUPLICATESTAGE_H
#define DANS02_DSTAGE_DUPLICATESTAGE_H

#include <memory>
#include <mutex>

#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

namespace dans {

template <typename T>
class DuplicateStage : public DStage<T> {
 public:
  DuplicateStage(Priority max_priority,
                 std::unique_ptr<Dispatcher<T>> dispatcher,
                 std::unique_ptr<Scheduler<T>> scheduler);
  ~DuplicateStage() {}

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
  MultiQueue<T> _multi_q;
  std::unique_ptr<Dispatcher<T>> _dispatcher;
  std::unique_ptr<Scheduler<T>> _scheduler;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_DUPLICATESTAGE_H