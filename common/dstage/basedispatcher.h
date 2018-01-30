#ifndef DANS02_DSTAGE_BASE_DISPATCHER_H
#define DANS02_DSTAGE_BASE_DISPATCHER_H

#include "common/dstage/basemultiqueue.h"
#include "common/dstage/job.h"

namespace dans {

template <typename T>
class BaseDispatcher {
 public:
  virtual ~BaseDispatcher(){};

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  virtual void Dispatch(UniqConstJobPtr<T> job_p,
                        unsigned requested_duplication) = 0;

  virtual void LinkMultiQ(BaseMultiQueue<T>* multi_q_p) = 0;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_BASE_DISPATCHER_H