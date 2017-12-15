
#include "common/dstage/dispatcher.h"

#include "common/dstage/job.h"

namespace duplicate_aware_scheduling {
template <typename T>
Dispatcher<T>::Dispatcher(unsigned max_duplication_level)
  :  _max_duplication_level(max_duplication_level) {}

// Introduces an ApplicationRequest to a DStage. base_prio is the incoming
// Priority of the ApplicationRequest. The Dispatcher will make
// duplication_level duplicates of the request for the Scheduler's use.
template <typename T>
bool Dispatcher<T>::Dispatch(Job<T> job)
{
  (void)job;
  // unsigned initial_priority = job;
  return true;
}

void LinkMultiQ();

} // namespace duplicate_aware_scheduling