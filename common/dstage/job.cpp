#include <common/dstage/job.h>

namespace duplicate_aware_scheduling {

JobIdFactory::JobIdFactory(unsigned seed) : _last_id(seed) {}

JobId JobIdFactory::CreateJobId() {
  _last_id++;
  return _last_id;
}

template <typename T>
Job<T>::Job(T job_data, JobId job_id, Priority priority,
            unsigned duplication_level)
    : job_data(job_data),
      job_id(job_id),
      priority(priority),
      duplication_level(duplication_level) {}

// Tests that two Jobs have the same JobId.
template <typename T>
bool Job<T>::operator==(const Job<T>& rhs) const {
  return rhs.job_id == job_id;
}

template struct Job<unsigned>;

}  // namespace duplicate_aware_scheduling