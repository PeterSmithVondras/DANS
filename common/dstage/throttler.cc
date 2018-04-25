#include "common/dstage/throttler.h"
#include "glog/logging.h"

namespace {}  // namespace

// template <typename T>
// ThrottleJob<T>::ThrottleJob(UniqJobPtr<T> job_in) {
//     : Job<T>(job_in->job_id, job_in->priority, job_in->duplication, std::move(job_in->job_data)),
//         total_completed(0) {}
// }

// template <typename T>
// UniqThrottleJobPtr<T> Throttler<T>::Dequeue(Priority prio) {
//   _multi_q_p->Dequeue(prio);
// }

// template class ThrottleJob<int>;
// template class Throttler<int>;

namespace dans {}  // namespace dans