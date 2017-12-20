#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE
namespace {
unsigned kNumberOfQueues = 3;

using namespace duplicate_aware_scheduling;
// using duplicate_aware_scheduling::Job;

}  // namespace

int main() {
  bool success = true;
  fprintf(stderr, "test_multiqueue... ");

  // JobIdFactory j_factory(0);
  // Job<unsigned> job(0, j_factory.CreateJobId(), 0, 3);

  MultiQueue prio_qs(kNumberOfQueues);

  prio_qs.Enqueue(100, std::vector<Priority> { 0, 1, 2 });

  // Test adding a Job
  // assert(mq.GetJobMap(job.job_id) == nullptr);
  // mq.AddJobMap(job);
  // assert(mq.GetJobMap(job.job_id) != nullptr);

  if (success) {
    printf(" Passed\n");
    return 0;
  } else
    printf(" Failed\n");
  return 1;
}