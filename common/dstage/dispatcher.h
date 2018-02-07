#ifndef DANS02_DSTAGE_DISPATCHER_H
#define DANS02_DSTAGE_DISPATCHER_H

#include "common/dstage/basedispatcher.h"
#include "common/dstage/basemultiqueue.h"
#include "common/dstage/job.h"

namespace dans {

template <typename T_INPUT, typename T_INTERNAL>
class Dispatcher : public BaseDispatcher<T_INPUT> {
 public:
  Dispatcher(Priority max_priority);

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  void Dispatch(UniqConstJobPtr<T_INPUT> job_p,
                unsigned requested_duplication) override;

  void LinkMultiQ(BaseMultiQueue<T_INTERNAL>* multi_q_p) override;

 protected:
  bool _running;
  const Priority _max_priority;
  BaseMultiQueue<T_INTERNAL>* _multi_q_p;

  virtual void DuplicateAndEnqueue(UniqConstJobPtr<T_INPUT> job_in,
                                   Priority max_prio, unsigned duplication) = 0;
};
}  // namespace dans

#include "common/dstage/dispatcher.hh"

#endif  // DANS02_DSTAGE_DISPATCHER_H