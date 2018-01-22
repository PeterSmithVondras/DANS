
#include "common/dstage/scheduler.h"

#include <cassert>

#include <memory>

namespace {}  // namespace

namespace dans {

template <typename T>
Scheduler<T>::Scheduler(unsigned max_priority)
    : _running(false),
      _max_priority(max_priority),
      _multi_q_p(nullptr),
      _destructing(false) {}

template <typename T>
Scheduler<T>::~Scheduler() {
  {
    std::unique_lock<std::shared_timed_mutex> lock(_destructing_lock);
    _destructing = true;
  }

  for (auto&& worker : _workers) {
    worker.join();
  }
}

template <typename T>
std::list<UniqConstJobPtr<T>> Scheduler<T>::Purge(JobId job_id) {
  static_cast<void>(job_id);
  std::list<UniqConstJobPtr<T>> purged;
  return purged;
}

template <typename T>
void Scheduler<T>::LinkMultiQ(MultiQueue<T>* multi_q_p) {
  assert(multi_q_p != nullptr);
  _multi_q_p = multi_q_p;
}

template <typename T>
void Scheduler<T>::Run() {
  assert(_multi_q_p != nullptr);
  assert(!_running);

  _running = true;
  for (Priority p = 0; p <= _max_priority; ++p) {
    _workers.push_back(std::thread(&Scheduler<T>::StartScheduling, this, p));
  }
}

template <typename T>
void Scheduler<T>::StartScheduling(Priority prio) {
  while (true) {
    {
      std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
      if (_destructing) return;
    }

    UniqConstJobPtr<T> job = _multi_q_p->Dequeue(prio);
    if (job == nullptr) continue;
  }
}

// As long as template implementation is in .cpp file, must explicitly tell
// compiler which types to compile...
template class Scheduler<JData>;

}  // namespace dans