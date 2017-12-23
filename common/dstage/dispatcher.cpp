
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
void Dispatcher<T>::Dispatch(std::unique_ptr<const Job<T>> job) {
  assert(_running);
  assert(job->priority <= _max_priority);

  std::shared_ptr<const Job<T>> duplicate_job(std::move(job));

  Priority max_prio =
      duplicate_job->priority + duplicate_job->requested_duplication >
              _max_priority
          ? _max_priority
          : duplicate_job->priority + duplicate_job->requested_duplication;

  for (Priority prio = duplicate_job->priority; prio <= max_prio; prio++) {
    _multi_q_p->Enqueue({duplicate_job->job_id, duplicate_job}, prio);
  }
}

template <typename T>
void Dispatcher<T>::LinkMultiQ(
    MultiQueue<JobId, std::shared_ptr<const Job<T>>>* multi_q_p) {
  assert(multi_q_p != nullptr);
  _multi_q_p = multi_q_p;
  _running = true;
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class Dispatcher<JData>;

}  // namespace duplicate_aware_scheduling