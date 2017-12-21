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

  prio_qs.Enqueue(100, std::vector<Priority>{0, 1, 2});

  prio_qs.Enqueue(200, std::vector<Priority>{0, 1, 2});

  prio_qs.Enqueue(300, std::vector<Priority>{0, 1, 2});

  prio_qs.Enqueue(0, std::vector<Priority>{0});
  prio_qs.Enqueue(1, std::vector<Priority>{1});
  prio_qs.Enqueue(2, std::vector<Priority>{2});

  prio_qs.Purge(100);
  prio_qs.Purge(300);

  for (unsigned i = 0; i < kNumberOfQueues; i++) {
    assert(prio_qs.Dequeue(i) == 200);
    assert(prio_qs.Dequeue(i) == i);
  }

  if (success) {
    printf(" Passed\n");
    return 0;
  } else
    printf(" Failed\n");
  return 1;
}