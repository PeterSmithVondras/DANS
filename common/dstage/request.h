#ifndef DANS02_DSTAGE_JOB_H
#define DANS02_DSTAGE_JOB_H

#include "dstage/priority.h"

namespace duplicate_aware_scheduling {

typedef const int JobId;

class JobIdFactory {
 public:
  JobIdFactory(int seed);
  JobId CreateJobId();
 private:
  int _next_id;
};

template <class T>
struct Job {
  Job(T job_data,
      JobId job_id,
      Priority priority);

  // Tests that two Jobs have the same JobId.
  bool operator==(Symbol& rhs) const;

  T job_data;
  JobId job_id;
  const Priority priority;
  uint duplication_level

};
} // namespace duplicate_aware_scheduling

#endif // DANS02_DSTAGE_JOB_H