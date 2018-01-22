
#include "common/dstage/scheduler.h"

#include <cassert>

#include <memory>

namespace {}  // namespace

namespace dans {

template <typename T>
Scheduler<T>::Scheduler(unsigned max_priority)
    : _running(false), _max_priority(max_priority), _multi_q_p(nullptr) {}

template <typename T>
std::list<UniqConstJobPtr<T>> Scheduler<T>::Purge(JobId job_id) {
  std::list<UniqConstJobPtr<T>> purged;
  return purged;
}

template <typename T>
void Scheduler<T>::LinkMultiQ(MultiQueue<T>* multi_q_p) {
  assert(multi_q_p != nullptr);
  _multi_q_p = multi_q_p;
  _running = true;
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class Scheduler<JData>;

}  // namespace dans