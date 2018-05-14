#ifndef DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H
#define DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H

#include "common/dstage/throttler.h"
#include "glog/logging.h"

namespace dans {
namespace {
int kThrottlerPriorities = 2;
const Priority kHighPriority = 0;
const Priority kLowPriority = 1;
}  // namespace

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
Throttler<T>::Throttler(BaseMultiQueue<T>* multi_q_p,
                        std::vector<int> throttle_targets)
    : _multi_q_p(multi_q_p),
      _scheduled_counts(_multi_q_p->MaxPriority() + 1, 0),
      _thread_waiting(_multi_q_p->MaxPriority() + 1, false),
      _throttle_targets(throttle_targets),
      _throttle_blocks(_multi_q_p->MaxPriority() + 1) {
  VLOG(4) << __PRETTY_FUNCTION__;
  CHECK(multi_q_p != nullptr)
      << "Attempted to instantiate Throttler with nullptr.";

  CHECK_EQ(_multi_q_p->MaxPriority() + 1, kThrottlerPriorities)
      << "Throttler is currently only set up for two priority levels, but "
         "there are "
      << _multi_q_p->MaxPriority() + 1 << " priorities in the MultiQueue.";
  CHECK_EQ(_throttle_targets.size(), kThrottlerPriorities)
      << "Throttler is currently only set up for two priority levels, but "
         "there are "
      << _throttle_targets.size() << " target throttle levels.";
}

template <typename T>
void Throttler<T>::IncrementJobCount(Priority prio) {
  ++_scheduled_counts[prio];
}

template <typename T>
void Throttler<T>::DecrementJobCount(Priority prio) {
  std::lock_guard<std::mutex> lock(_state_lock);
  --_scheduled_counts[prio];

  DecideToSchedule();
}

template <typename T>
std::unique_ptr<typename Throttler<T>::ThrottleJob> Throttler<T>::Dequeue(
    Priority prio) {
  std::unique_ptr<Throttler<T>::ThrottleJob> ret = nullptr;
  _throttle_blocks[prio].lock();
  {
    std::lock_guard<std::mutex> lock(_state_lock);
    _thread_waiting[prio] = true;
  }

  auto job = _multi_q_p->Dequeue(prio);

  std::lock_guard<std::mutex> lock(_state_lock);
  _thread_waiting[prio] = false;
  if (job != nullptr) {
    ret = std::make_unique<Throttler<T>::ThrottleJob>(std::move(job), this);
  }
  DecideToSchedule();
  return ret;
}

// TODO: this really complicated function needs to be smoothed out.
template <typename T>
void Throttler<T>::DecideToSchedule() {
  // adjust high priority
  if (_scheduled_counts[kHighPriority] < _throttle_targets[kHighPriority] &&
      !_thread_waiting[kHighPriority]) {
    _throttle_blocks[kHighPriority].unlock();
  }
  // adjust low priority
  int high_prio_jobs =
      _scheduled_counts[kHighPriority] + _multi_q_p->Size(kHighPriority);
  if (high_prio_jobs >= _throttle_targets[kHighPriority]) {
    if (_thread_waiting[kLowPriority]) _multi_q_p->ReleaseOne(kLowPriority);
  } else if (_scheduled_counts[kLowPriority] <
                 _throttle_targets[kLowPriority] &&
             !_thread_waiting[kLowPriority]) {
    _throttle_blocks[kLowPriority].unlock();
  }
}

template <typename T>
std::string Throttler<T>::Describe() {
  std::ostringstream description;
  description << "Jobs_Scheduled=(";
  for (const auto& jobs_at_this_prio : _scheduled_counts) {
    description << std::to_string(jobs_at_this_prio) << "_";
  }
  description << ") ";
  return description.str();
}

}  // namespace dans

#endif  // DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H