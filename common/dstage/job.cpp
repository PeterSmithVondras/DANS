#include <common/dstage/job.h>

namespace duplicate_aware_scheduling {

JobIdFactory::JobIdFactory(unsigned seed) : _last_id(seed) {}

JobId JobIdFactory::CreateJobId() {
  _last_id++;
  return _last_id;
}

template <typename T>
Job<T>::Job(T job_data, JobId job_id, Priority priority,
            unsigned requested_duplication)
    : job_data(job_data),
      job_id(job_id),
      priority(priority),
      requested_duplication(requested_duplication) {}

template struct Job<unsigned>;

}  // namespace duplicate_aware_scheduling