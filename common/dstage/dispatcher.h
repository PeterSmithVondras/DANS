#ifndef DANS02_DSTAGE_DISPATCHER_H
#define DANS02_DSTAGE_DISPATCHER_H

#include <mutex>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace dans {

template <typename T>
class Dispatcher {
 public:
  Dispatcher(Priority max_priority);

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  void Dispatch(UniqConstJobPtr<T> job, unsigned requested_duplication);

  void LinkMultiQ(MultiQueue<T>* multi_q_p);

 private:
  bool _running;
  const Priority _max_priority;
  MultiQueue<T>* _multi_q_p;
};
}  // namespace dans

#endif  // DANS02_DSTAGE_DISPATCHER_H