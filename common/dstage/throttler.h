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

  Throttler(BaseMultiQueue<T>* multi_q_p);

  // Thread safe and blocking dequeue function will dequeue from the queue
  // associated to "prio."
  std::unique_ptr<typename Throttler<T>::ThrottleJob> Dequeue(Priority prio);

  std::string Describe();

  void IncrementJobCount(Priority prio);
  void DecrementJobCount(Priority prio);

 private:
  BaseMultiQueue<T>* _multi_q_p;
  // Counters are in unique_ptr's as mutex's cannot be moved.
  std::vector<std::unique_ptr<Counter>> _jobs_scheduled;
};

template <typename T>
using UniqThrottleJobPtr = std::unique_ptr<typename Throttler<T>::ThrottleJob>;
template <typename T>
using SharedThrottleJobPtr =
    std::shared_ptr<typename Throttler<T>::ThrottleJob>;

}  // namespace dans

#include "common/dstage/throttler.hh"

#endif  // COMMON_DSTAGE_THROTTLER