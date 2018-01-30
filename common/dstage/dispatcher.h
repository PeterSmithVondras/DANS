#ifndef DANS02_DSTAGE_DISPATCHER_H
#define DANS02_DSTAGE_DISPATCHER_H

#include "common/dstage/basedispatcher.h"
#include "common/dstage/basemultiqueue.h"
#include "common/dstage/job.h"

namespace dans {

template <typename T>
class Dispatcher : public BaseDispatcher<T> {
 public:
  Dispatcher(Priority max_priority);

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  void Dispatch(UniqConstJobPtr<T> job_p,
                unsigned requested_duplication) override;

  void LinkMultiQ(BaseMultiQueue<T>* multi_q_p) override;

 private:
  bool _running;
  const Priority _max_priority;
  BaseMultiQueue<T>* _multi_q_p;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_DISPATCHER_H