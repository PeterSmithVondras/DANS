#ifndef DANS02_DSTAGE_JOB_H
#define DANS02_DSTAGE_JOB_H

#include <vector>
#include "common/dstage/priority.h"

namespace duplicate_aware_scheduling {

typedef int JobId;

class JobIdFactory {
 public:
  JobIdFactory(int seed);
  JobId CreateJobId();
 private:
  int _last_id;
};

template <typename T>
struct Job {
  Job(T job_data,
      JobId job_id,
      Priority priority);

  // Tests that two Jobs have the same JobId.
  bool operator==(const Job<T>& rhs) const;

  T job_data;
  const JobId job_id;
  const Priority priority;
  unsigned duplication_level;

};

template <typename T>
struct JobMap {
  Job<T> job;
  std::vector<JobMap**> instances_of_this_job;
};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_JOB_H