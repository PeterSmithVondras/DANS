
#include "common/dstage/dispatcher.h"

#include <cassert>
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
void Dispatcher<T>::Dispatch(Job<T> job) {
  assert(_running);
  assert(job.priority <= _max_priority);

  std::vector<Priority> dupes;

  Priority max_prio = job.priority + job.requested_duplication > _max_priority
                          ? _max_priority
                          : job.priority + job.requested_duplication;

  for (Priority dup = job.priority; dup <= max_prio; dup++)
    dupes.push_back(dup);
  _multi_q_p->Enqueue(job.job_id, dupes);
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