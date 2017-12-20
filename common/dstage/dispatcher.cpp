
#include "common/dstage/dispatcher.h"

#include <mutex>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace {}  // namespace

namespace duplicate_aware_scheduling {

template <typename T>
Dispatcher<T>::Dispatcher(unsigned max_duplication_level)
    : _max_duplication_level(max_duplication_level),
      _q_mutex_p(nullptr),
      _multi_q_p(nullptr) {}

// Duplicates job and inserts into MultiQueue
template <typename T>
bool Dispatcher<T>::Dispatch(Job<T> job) {
  if (_q_mutex_p == nullptr || _multi_q_p == nullptr) return false;

  std::lock_guard<std::mutex> guard(*_q_mutex_p);
  // unique_ptr<struct JobMap<T>>

  (void)job;
  // unsigned initial_priority = job;
  return true;
}

template <typename T>
void Dispatcher<T>::LinkMultiQ(MultiQueue* multi_q_p, std::mutex* q_mutex_p) {
  _multi_q_p = multi_q_p;
  _q_mutex_p = q_mutex_p;
}

template class Dispatcher<unsigned>;

}  // namespace duplicate_aware_scheduling