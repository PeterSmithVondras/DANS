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
             job_in->duplication, job_in->start_time),
      _throttler(throttler),
      total_completed(0) {
  VLOG(4) << __PRETTY_FUNCTION__;
  _throttler->IncrementJobCount(Job<T>::priority);
}

template <typename T>
Throttler<T>::ThrottleJob::~ThrottleJob() {
  VLOG(4) << __PRETTY_FUNCTION__;
  _throttler->DecrementJobCount(Job<T>::priority);
  _throttler->AddStat(
      Job<T>::priority, total_completed,
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::high_resolution_clock::now() - Job<T>::start_time));
}

template <typename T>
void Throttler<T>::ThrottleJob::ReportThroughput(unsigned new_units_completed) {
  total_completed += new_units_completed;
}

template <typename T>
void Throttler<T>::AddStat(Priority prio, unsigned total_completed,
                           std::chrono::milliseconds lat) {
  std::shared_lock<std::shared_timed_mutex> lock(_updater_lock);
  if (prio == 0) {
    std::lock_guard<std::mutex> lock_prim(_updater_primary_lock);
    _primary_latencies.push_back(lat);
    _primary_sizes.push_back(total_completed);
  } else {
    std::lock_guard<std::mutex> lock_sec(_updater_secondary_lock);
    _secondary_latencies.push_back(lat);
    _secondary_sizes.push_back(total_completed);
  }
}

template <typename T>
Throttler<T>::Throttler(BaseMultiQueue<T>* multi_q_p,
                        std::array<int, kThrottlerPriorities> throttle_targets)
    : _multi_q_p(multi_q_p),
      _throttle_targets(std::move(throttle_targets)),
      _joining(false) {
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

  RunEveryT(std::chrono::milliseconds(500), [](Throttler<T>* th, Stats&& state,
                                               std::chrono::milliseconds time) {
    if (state.primary_latencies.size() > 0)
      LOG(WARNING) << "RUNEVERYT -- prim lat: "
                   << state.primary_latencies[0].count() << ", "
                   << state.primary_latencies.size();
  });
}

template <typename T>
Throttler<T>::~Throttler() {
  StopRunningEveryT();
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
  auto job = _multi_q_p->Dequeue(prio);
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
  if (prio == 0) {
    _primary_waiting = false;
  } else {
    _secondary_waiting = false;
  }

  DecideToScheduleAfterScheduling();
}

// TODO: this really complicated function needs to be smoothed out.
template <typename T>
void Throttler<T>::DecideToScheduleAfterScheduling() {
  int high_prio_jobs = _scheduled_counts[kHighPriority]->Count() +
                       _multi_q_p->Size(kHighPriority);
  bool attempted_to_schedule_primary = false;
  // adjust high priority
  if (_scheduled_counts[kHighPriority]->Count() <
          _throttle_targets[kHighPriority] &&
      !_primary_waiting) {
    _primary_waiting = true;
    _throttle_blocks[kHighPriority].unlock();
    attempted_to_schedule_primary = true;
  }

  // adjust low priority
  if (high_prio_jobs >= _throttle_targets[kHighPriority]) {
    if (_secondary_waiting) _multi_q_p->ReleaseOne(kLowPriority);
  } else if (_scheduled_counts[kLowPriority]->Count() <
                 _throttle_targets[kLowPriority] &&
             !_secondary_waiting && !attempted_to_schedule_primary) {
    _secondary_waiting = true;
    _throttle_blocks[kLowPriority].unlock();
  }
}

// TODO: this really complicated function needs to be smoothed out.
template <typename T>
void Throttler<T>::DecideToScheduleCompleting(Priority prio) {
  std::lock_guard<std::mutex> lock(_state_lock);
  int high_prio_jobs = _scheduled_counts[kHighPriority]->Count() +
                       _multi_q_p->Size(kHighPriority);
  bool attempted_to_schedule_primary = false;
  // adjust high priority
  if (_scheduled_counts[kHighPriority]->Count() <
          _throttle_targets[kHighPriority] &&
      !_primary_waiting) {
    _primary_waiting = true;
    _throttle_blocks[kHighPriority].unlock();
    attempted_to_schedule_primary = true;
  }

  // adjust low priority
  if (high_prio_jobs < _throttle_targets[kHighPriority] &&
      _scheduled_counts[kLowPriority]->Count() <
          _throttle_targets[kLowPriority] &&
      !_secondary_waiting && !attempted_to_schedule_primary) {
    _secondary_waiting = true;
    _throttle_blocks[kLowPriority].unlock();
  }
}

template <typename T>
void Throttler<T>::SetThrottle(Priority prio, int threshold) {
  std::lock_guard<std::mutex> lock(_state_lock);
  _throttle_targets[prio] = threshold;
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

template <typename T>
void Throttler<T>::StopRunningEveryT() {
  _joining = true;
  if (_updater_thread != nullptr) _updater_thread->join();
}

template <typename T>
void Throttler<T>::RunEveryT(
    std::chrono::milliseconds t,
    std::function<void(Throttler<T>*, Stats&&, std::chrono::milliseconds)> cb) {
  StopRunningEveryT();
  _joining = false;
  _updater_thread = std::make_unique<std::thread>(
      std::bind(&Throttler<T>::Runner, this, std::placeholders::_1,
                std::placeholders::_2),
      t, cb);
}

template <typename T>
void Throttler<T>::Runner(
    std::chrono::milliseconds t,
    std::function<void(Throttler<T>*, Stats&&, std::chrono::milliseconds time)>
        cb) {
  Stats stats;
  std::chrono::high_resolution_clock::time_point start, end;
  {
    std::unique_lock<std::shared_timed_mutex> lock(_updater_lock);
    _primary_latencies.clear();
    _secondary_latencies.clear();
    _primary_sizes.clear();
    _secondary_sizes.clear();
  }
  while (true) {
    start = std::chrono::high_resolution_clock::now();
    if (_joining.load()) return;
    std::this_thread::sleep_for(t);
    {
      std::unique_lock<std::shared_timed_mutex> lock(_updater_lock);
      end = std::chrono::high_resolution_clock::now();
      stats = {_primary_latencies, _primary_sizes, _secondary_latencies,
               _secondary_sizes};
      _primary_latencies.clear();
      _secondary_latencies.clear();
      _primary_sizes.clear();
      _secondary_sizes.clear();
    }
    cb(this, std::move(stats),
       std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
  }
}

}  // namespace dans

#endif  // DANS02_COMMON_DSTAGE_THROTTLER_IMPLEMENTATION_H