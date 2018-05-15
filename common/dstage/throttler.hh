#ifndef DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H
#define DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H

#include "common/dstage/throttler.h"
#include "glog/logging.h"

namespace dans {
namespace {
const int kThrottlerPriorities = 2;
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
                        std::array<int, kThrottlerPriorities> throttle_targets)
    : _multi_q_p(multi_q_p), _throttle_targets(std::move(throttle_targets)) {
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

  for (int i = 0; i < kThrottlerPriorities; i++) {
    _scheduled_counts.push_back(std::make_unique<Counter>(0));
  }
}

template <typename T>
void Throttler<T>::IncrementJobCount(Priority prio) {
  _scheduled_counts[prio]->Increment();
}

template <typename T>
void Throttler<T>::DecrementJobCount(Priority prio) {
  _scheduled_counts[prio]->Decrement();
  DecideToScheduleCompleting(prio);
}

template <typename T>
std::unique_ptr<typename Throttler<T>::ThrottleJob> Throttler<T>::Dequeue(
    Priority prio) {
  std::unique_ptr<Throttler<T>::ThrottleJob> ret_val = nullptr;
  _throttle_blocks[prio].lock();
  if (prio == 0) {
    _primary_waiting = true;
  } else {
    _secondary_waiting = true;
  }

  auto job = _multi_q_p->Dequeue(prio);

  if (prio == 0) {
    _primary_waiting = false;
  } else {
    _secondary_waiting = false;
  }
  if (job != nullptr) {
    ret_val = std::make_unique<Throttler<T>::ThrottleJob>(std::move(job), this);
  }
  DecideToScheduleAfterScheduling(prio);
  return ret_val;
}

// TODO: this really complicated function needs to be smoothed out.
template <typename T>
void Throttler<T>::DecideToScheduleAfterScheduling(Priority prio) {
  std::lock_guard<std::mutex> lock(_state_lock);
  int high_prio_jobs = _scheduled_counts[kHighPriority]->Count() +
                       _multi_q_p->Size(kHighPriority);
  if (prio == kHighPriority) {
    // adjust high priority
    if (_scheduled_counts[kHighPriority]->Count() <
            _throttle_targets[kHighPriority] &&
        !_primary_waiting.load()) {
      _throttle_blocks[kHighPriority].unlock();
    }

    // adjust low priority
    if (high_prio_jobs >= _throttle_targets[kHighPriority] &&
        _secondary_waiting.load()) {
      _multi_q_p->ReleaseOne(kLowPriority);
    }
  } else {
    // Called by low priority.
    if (high_prio_jobs < _throttle_targets[kHighPriority] &&
        _scheduled_counts[kLowPriority]->Count() <
            _throttle_targets[kLowPriority] &&
        !_secondary_waiting.load()) {
      _throttle_blocks[kLowPriority].unlock();
    }
  }
}

// TODO: this really complicated function needs to be smoothed out.
template <typename T>
void Throttler<T>::DecideToScheduleCompleting(Priority prio) {
  std::lock_guard<std::mutex> lock(_state_lock);
  int high_prio_jobs = _scheduled_counts[kHighPriority]->Count() +
                       _multi_q_p->Size(kHighPriority);
  if (prio == kHighPriority) {
    // adjust high priority
    if (_scheduled_counts[kHighPriority]->Count() <
            _throttle_targets[kHighPriority] &&
        !_primary_waiting.load()) {
      _throttle_blocks[kHighPriority].unlock();
    }

    // adjust low priority
    if (high_prio_jobs >= _throttle_targets[kHighPriority] &&
        _secondary_waiting.load()) {
      _multi_q_p->ReleaseOne(kLowPriority);
    } else if (!_secondary_waiting.load()) {
      _throttle_blocks[kLowPriority].unlock();
    }
  } else {
    // Called by low priority.
    if (high_prio_jobs < _throttle_targets[kHighPriority] &&
        _scheduled_counts[kLowPriority]->Count() <
            _throttle_targets[kLowPriority] &&
        !_secondary_waiting.load()) {
      _throttle_blocks[kLowPriority].unlock();
    }
  }
}

template <typename T>
std::string Throttler<T>::Describe() {
  std::ostringstream description;
  description << "Jobs_Scheduled=(";
  for (const auto& jobs_at_this_prio : _scheduled_counts) {
    description << std::to_string(jobs_at_this_prio->Count()) << "_";
  }
  description << ") ";
  return description.str();
}

}  // namespace dans

#endif  // DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H