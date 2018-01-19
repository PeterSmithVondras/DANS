
#include "common/dstage/dispatcher.h"

#include <cassert>

#include <memory>
#include <vector>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace {}  // namespace

namespace duplicate_aware_scheduling {

template <typename T>
Dispatcher<T>::Dispatcher(unsigned max_priority)
    : _running(false), _max_priority(max_priority), _multi_q_p(nullptr) {}

// Duplicates job and inserts into MultiQueue
template <typename T>
void Dispatcher<T>::Dispatch(UniqConstJobPtr<T> job,
                             unsigned requested_duplication) {
  assert(_running);
  assert(job->priority <= _max_priority);

  // Get lowest priority (highest value) appropriate.
  Priority max_prio = job->priority + requested_duplication > _max_priority
                          ? _max_priority
                          : job->priority + requested_duplication;
  unsigned duplication = max_prio - job->priority;

  for (Priority prio = job->priority; prio <= max_prio; prio++) {
    auto duplicate_job = std::make_unique<const Job<T>>(
        job->job_data, job->job_id, prio, duplication);
    _multi_q_p->Enqueue(std::move(duplicate_job));
  }
}

template <typename T>
void Dispatcher<T>::LinkMultiQ(MultiQueue<T>* multi_q_p) {
  assert(multi_q_p != nullptr);
  _multi_q_p = multi_q_p;
  _running = true;
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class Dispatcher<JData>;

}  // namespace duplicate_aware_scheduling