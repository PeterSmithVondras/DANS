#ifndef DANS02_DSTAGE_DISPATCHER_H
#define DANS02_DSTAGE_DISPATCHER_H

#include <mutex>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace duplicate_aware_scheduling {

template <typename T>
class Dispatcher {
 public:
  Dispatcher(unsigned max_duplication_level);

  // Introduces an ApplicationRequest to a DStage. base_prio is the incoming
  // Priority of the ApplicationRequest. The Dispatcher will make
  // duplication_level duplicates of the request for the Scheduler's use.
  bool Dispatch(Job<T> job);

  void LinkMultiQ(MultiQueue* multi_q_p, std::mutex* q_mutex_p);

 private:
  const unsigned _max_duplication_level;
  std::mutex* _q_mutex_p;
  MultiQueue* _multi_q_p;
};
}  // namespace duplicate_aware_scheduling

#endif  // DANS02_DSTAGE_DISPATCHER_H