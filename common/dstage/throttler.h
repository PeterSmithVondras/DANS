#ifndef DANS02_CLIENT_CONNECT_HANDLER_H
#define DANS02_CLIENT_CONNECT_HANDLER_H

#include <memory>
#include <mutex>
#include <shared_mutex>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace dans {

template <typename T>
class ThrottleJob : public Job<T> {
 public:
  ThrottleJob(T job_data, JobId job_id, Priority priority, unsigned duplication)
      : Job<T>(job_id, priority, duplication, std::move(job_data)),
        total_completed(0) {}

  void ReportThroughput(unsigned new_units_completed);

 private:
  unsigned total_completed;
  // Throttler* _throttler;
};

template <typename T>
using UniqThrottleJobPtr = std::unique_ptr<ThrottleJob<T>>;

template <typename T>
class Throttler {
 public:
  Throttler(BaseMultiQueue<T>* multi_q_p) : _multi_q_p(multi_q_p) {}

  // Thread safe and blocking dequeue function will dequeue from the queue
  // associated to "prio."
  UniqThrottleJobPtr<T> Dequeue(Priority prio);

 private:
  BaseMultiQueue<T>* _multi_q_p;
};

}  // namespace dans

#endif  // DANS02_CLIENT_CONNECT_HANDLER_H