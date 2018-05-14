#ifndef COMMON_DSTAGE_THROTTLER
#define COMMON_DSTAGE_THROTTLER

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

  Throttler(BaseMultiQueue<T>* multi_q_p, std::vector<int> throttle_targets);

  // Thread safe and blocking dequeue function will dequeue from the queue
  // associated to "prio."
  std::unique_ptr<typename Throttler<T>::ThrottleJob> Dequeue(Priority prio);

  std::string Describe();

 private:
  BaseMultiQueue<T>* _multi_q_p;

  std::mutex _state_lock;
  std::vector<int> _scheduled_counts;
  std::vector<bool> _thread_waiting;
  std::vector<int> _throttle_targets;
  std::vector<std::mutex> _throttle_blocks;

  // Not a thread safe function.
  void IncrementJobCount(Priority prio);
  // This IS a thread safe function.
  void DecrementJobCount(Priority prio);
  void DecideToSchedule();
};

template <typename T>
using UniqThrottleJobPtr = std::unique_ptr<typename Throttler<T>::ThrottleJob>;
template <typename T>
using SharedThrottleJobPtr =
    std::shared_ptr<typename Throttler<T>::ThrottleJob>;

}  // namespace dans

#include "common/dstage/throttler.hh"

#endif  // COMMON_DSTAGE_THROTTLER