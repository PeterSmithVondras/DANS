#ifndef DANS02_DSTAGE_JOB_H
#define DANS02_DSTAGE_JOB_H

#include <memory>
#include <string>
#include <vector>
#include "common/dstage/priority.h"

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
    _last_id++;
    return _last_id;
  }

 private:
  JobId _last_id;
};

template <typename T>
struct Job {
  Job(T job_data, JobId job_id, Priority priority, unsigned duplication)
      : job_id(job_id),
        priority(priority),
        duplication(duplication),
        job_data(std::move(job_data)) {}

  bool operator==(const Job& rhs) const { return job_id == rhs.job_id; }
  bool operator!=(const Job& rhs) const { return !operator==(rhs); }

  JobId job_id;
  Priority priority;
  unsigned duplication;
  T job_data;
};

template <typename T>
using UniqJobPtr = std::unique_ptr<Job<T>>;

template <typename T>
using SharedJobPtr = std::shared_ptr<Job<T>>;

}  // namespace dans

#endif  // DANS02_DSTAGE_JOB_H