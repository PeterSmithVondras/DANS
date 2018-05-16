#ifndef COMMON_DSTAGE_THROTTLER
#define COMMON_DSTAGE_THROTTLER

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/synchronization.h"

namespace dans {

template <typename T>
class Throttler {
  struct Stats {
    std::vector<std::chrono::milliseconds> primary_latencies;
    std::vector<int> primary_sizes;
    std::vector<std::chrono::milliseconds> secondary_latencies;
    std::vector<int> secondary_sizes;
  };

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
  ~Throttler();

  // Thread safe and blocking dequeue function will dequeue from the queue
  // associated to "prio."
  std::unique_ptr<typename Throttler<T>::ThrottleJob> Dequeue(Priority prio);

  void RunEveryT(
      std::chrono::milliseconds t,
      std::function<void(Throttler<T>*, Stats&&, std::chrono::milliseconds)>
          cb);
  void SetThrottle(Priority prio, int threshold);

  std::string Describe();

 private:
  BaseMultiQueue<T>* _multi_q_p;

  std::array<int, 2> _throttle_targets;
  std::array<std::mutex, 2> _throttle_blocks;

  std::vector<std::unique_ptr<Counter>> _scheduled_counts;
  // TODO: Move back to array of bools.
  bool _primary_waiting = false;
  bool _secondary_waiting = false;
  std::vector<std::chrono::milliseconds> _primary_latencies;
  std::vector<std::chrono::milliseconds> _secondary_latencies;
  std::vector<int> _primary_sizes;
  std::vector<int> _secondary_sizes;

  std::mutex _state_lock;

  std::shared_timed_mutex _updater_lock;
  std::mutex _updater_primary_lock;
  std::mutex _updater_secondary_lock;
  std::unique_ptr<std::thread> _updater_thread;
  std::atomic<bool> _joining;

  void IncrementJobCount(Priority prio);
  void DecrementJobCount(Priority prio);
  void DecideToScheduleAfterScheduling(Priority prio);
  void DecideToScheduleAfterScheduling();
  void DecideToScheduleCompleting(Priority prio);
  void StopRunningEveryT();
  void Runner(
      std::chrono::milliseconds t,
      std::function<void(Throttler<T>*, Stats&&, std::chrono::milliseconds)>
          cb);
  void AddStat(Priority prio, unsigned total_completed,
               std::chrono::milliseconds lat);
};

template <typename T>
using UniqThrottleJobPtr = std::unique_ptr<typename Throttler<T>::ThrottleJob>;
template <typename T>
using SharedThrottleJobPtr =
    std::shared_ptr<typename Throttler<T>::ThrottleJob>;

}  // namespace dans

#include "common/dstage/throttler.hh"

#endif  // COMMON_DSTAGE_THROTTLER