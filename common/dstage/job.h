#ifndef DANS02_DSTAGE_JOB_H
#define DANS02_DSTAGE_JOB_H

#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <vector>
#include "common/dstage/priority.h"
#include "common/dstage/synchronization.h"

namespace dans {
// This is an example of a specific data type for a specific dstage.
struct JData {
  JData(unsigned foo) : foo(foo) {}
  unsigned foo;
};

typedef unsigned JobId;

class JobIdFactory {
 public:
  JobIdFactory(unsigned seed) : _last_id(seed) {}
  JobId CreateJobId() {
    _last_id.Increment();
    return _last_id.Count();
  }

 private:
  Counter _last_id;
};

template <typename T>
class Job {
 public:
  Job(T job_data, JobId job_id, Priority priority, unsigned duplication)
      : job_id(job_id),
        priority(priority),
        duplication(duplication),
        start_time(std::chrono::high_resolution_clock::now()),
        job_data(std::move(job_data)) {}

  Job(T job_data, JobId job_id, Priority priority, unsigned duplication,
      std::chrono::high_resolution_clock::time_point start_time)
      : job_id(job_id),
        priority(priority),
        duplication(duplication),
        start_time(start_time),
        job_data(std::move(job_data)) {}

  bool operator==(const Job& rhs) const { return job_id == rhs.job_id; }
  bool operator!=(const Job& rhs) const { return !operator==(rhs); }

  std::string Describe() {
    std::string description;
    description =
        "job_id=" + std::to_string(job_id) +
        " prio=" + std::to_string(priority) + " duration=" +
        std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start_time)
                .count()) +
        "ms";
    return description;
  }

  JobId job_id;
  Priority priority;
  unsigned duplication;
  std::chrono::high_resolution_clock::time_point start_time;
  T job_data;
};

template <typename T>
using UniqJobPtr = std::unique_ptr<Job<T>>;

template <typename T>
using SharedJobPtr = std::shared_ptr<Job<T>>;

}  // namespace dans

#endif  // DANS02_DSTAGE_JOB_H