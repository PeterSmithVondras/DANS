#ifndef DANS02_DSTAGE_DSTAGE_H
#define DANS02_DSTAGE_DSTAGE_H

#include <memory>

#include "common/dstage/basedispatcher.h"
#include "common/dstage/basedstage.h"
#include "common/dstage/basemultiqueue.h"
#include "common/dstage/basescheduler.h"
#include "common/dstage/job.h"
#include "common/dstage/priority.h"

namespace dans {

template <typename T_INPUT, typename T_INTERNAL>
class DStage : public BaseDStage<T_INPUT> {
 public:
  DStage(Priority max_priority,
         std::unique_ptr<BaseMultiQueue<T_INTERNAL>> multi_q,
         std::unique_ptr<BaseDispatcher<T_INPUT, T_INTERNAL>> dispatcher,
         std::unique_ptr<BaseScheduler<T_INTERNAL>> scheduler);
  ~DStage() {}

  // Introduces an ApplicationJob to a DStage. base_prio is the incoming
  // Priority of the ApplicationJob. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  void Dispatch(UniqJobPtr<T_INPUT> job_p,
                unsigned requested_duplication) override;

  // Purge will attempt to remove all instances of the Job linked to job_id in
  // the Dispatcher, Scheduler and forward the request on to any linked DStages.
  unsigned Purge(JobId job_id) override;

 protected:
  const Priority _max_priority;
  std::unique_ptr<BaseMultiQueue<T_INTERNAL>> _multi_q;
  std::unique_ptr<BaseDispatcher<T_INPUT, T_INTERNAL>> _dispatcher;
  std::unique_ptr<BaseScheduler<T_INTERNAL>> _scheduler;
};
}  // namespace dans

#include "common/dstage/dstage.hh"

#endif  // DANS02_DSTAGE_DSTAGE_H