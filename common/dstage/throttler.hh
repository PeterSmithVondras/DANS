#ifndef DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H
#define DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H

#include "common/dstage/throttler.h"
#include "glog/logging.h"

namespace dans {

template <typename T>
Throttler<T>::ThrottleJob::ThrottleJob(UniqJobPtr<T>&& job_in,
                                       Throttler<T>* throttler)
    : Job<T>(std::move(job_in->job_data), job_in->job_id, job_in->priority,
             job_in->duplication),
      _throttler(throttler),
      total_completed(0) {
  VLOG(4) << __PRETTY_FUNCTION__;
  _throttler->IncrementJobCount(Job<T>::priority);
}

template <typename T>
Throttler<T>::ThrottleJob::~ThrottleJob() {
  VLOG(4) << __PRETTY_FUNCTION__;
  _throttler->DecrementJobCount(Job<T>::priority);
}

template <typename T>
Throttler<T>::Throttler(BaseMultiQueue<T>* multi_q_p) : _multi_q_p(multi_q_p) {
  VLOG(4) << __PRETTY_FUNCTION__;
  CHECK(multi_q_p != nullptr)
      << "Attempted to instantiate Throttler with nullptr.";
  for (unsigned i = 0; i < _multi_q_p->MaxPriority() + 1; i++) {
    _jobs_scheduled.push_back(std::make_unique<Counter>(0));
  }
}

template <typename T>
void Throttler<T>::IncrementJobCount(Priority prio) {
  _jobs_scheduled[prio]->Increment();
}

template <typename T>
void Throttler<T>::DecrementJobCount(Priority prio) {
  _jobs_scheduled[prio]->Decrement();
}

template <typename T>
std::unique_ptr<typename Throttler<T>::ThrottleJob> Throttler<T>::Dequeue(
    Priority prio) {
  auto job = _multi_q_p->Dequeue(prio);
  if (job == nullptr) {
    return nullptr;
  }

  auto throttle_job =
      std::make_unique<Throttler<T>::ThrottleJob>(std::move(job), this);
  return throttle_job;
}

template <typename T>
std::string Throttler<T>::Describe() {
  std::ostringstream description;
  description << "Jobs_Scheduled=(";
  for (const auto& jobs_at_this_prio : _jobs_scheduled) {
    description << std::to_string(jobs_at_this_prio->Count()) << "_";
  }
  description << ") ";
  return description.str();
}

}  // namespace dans

#endif  // DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H