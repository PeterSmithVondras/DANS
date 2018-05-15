#ifndef COMMON_DSTAGE_THROTTLER
#define COMMON_DSTAGE_THROTTLER

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/synchronization.h"

namespace dans {

template <typename T>
class Throttler {
 public:
  class ThrottleJob : public Job<T> {
   public:
    ThrottleJob() = delete;
    ThrottleJob(UniqJobPtr<T>&& job_in, Throttler<T>* throttler);
    ~ThrottleJob();

    void ReportThroughput(unsigned new_units_completed);

   private:
    friend class Throttler;

    Throttler<T>* _throttler;
    unsigned total_completed;
  };

  Throttler(BaseMultiQueue<T>* multi_q_p, std::array<int, 2> throttle_targets);

  // Thread safe and blocking dequeue function will dequeue from the queue
  // associated to "prio."
  std::unique_ptr<typename Throttler<T>::ThrottleJob> Dequeue(Priority prio);

  std::string Describe();

 private:
  BaseMultiQueue<T>* _multi_q_p;

  std::array<int, 2> _throttle_targets;
  std::array<std::mutex, 2> _throttle_blocks;

  std::vector<std::unique_ptr<Counter>> _scheduled_counts;
  // TODO: Figure out why it was so difficult to have these atomics in a
  // std::array. Also, figure out if it would be cleaner and or more correct to
  // use std::condition_variable instead.
  bool _primary_waiting = false;
  bool _secondary_waiting = false;

  std::mutex _state_lock;

  // Not a thread safe function.
  void IncrementJobCount(Priority prio);
  // This IS a thread safe function.
  void DecrementJobCount(Priority prio);
  void DecideToScheduleAfterScheduling(Priority prio);
  void DecideToScheduleAfterScheduling();
  void DecideToScheduleCompleting(Priority prio);
};

template <typename T>
using UniqThrottleJobPtr = std::unique_ptr<typename Throttler<T>::ThrottleJob>;
template <typename T>
using SharedThrottleJobPtr =
    std::shared_ptr<typename Throttler<T>::ThrottleJob>;

}  // namespace dans

#include "common/dstage/throttler.hh"

#endif  // COMMON_DSTAGE_THROTTLER