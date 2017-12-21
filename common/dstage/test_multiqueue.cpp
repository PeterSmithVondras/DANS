#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE

namespace {

using duplicate_aware_scheduling::MultiQueue;
using duplicate_aware_scheduling::Priority;

unsigned kNumberOfQueues = 3;
unsigned kEmpty = 0;
unsigned kDecoyA = 500;
unsigned kDecoyB = kDecoyA + 1;
unsigned kRealValue = kDecoyB + 1;

}  // namespace

int main() {
  bool success = true;
  fprintf(stderr, "test_multiqueue... ");

  MultiQueue prio_qs(kNumberOfQueues);

  // Purge a missing JobId returns empty list.
  assert(prio_qs.Purge(kDecoyA).size() == kEmpty);

  // Purge a job with several instances.
  prio_qs.Enqueue(kDecoyA, std::vector<Priority>{0, 1, 2});
  assert(prio_qs.Purge(kDecoyA).size() == kNumberOfQueues);

  // Add some JobIds with some garbage in between.
  prio_qs.Enqueue(kDecoyA, std::vector<Priority>{0, 1, 2});
  prio_qs.Enqueue(kRealValue, std::vector<Priority>{0, 1, 2});
  prio_qs.Enqueue(kDecoyB, std::vector<Priority>{0, 1, 2});
  prio_qs.Enqueue(0, std::vector<Priority>{0});
  prio_qs.Enqueue(1, std::vector<Priority>{1});
  prio_qs.Enqueue(2, std::vector<Priority>{2});

  // Purge the garbage.
  assert(prio_qs.Purge(kDecoyA).size() == kNumberOfQueues);
  assert(prio_qs.Purge(kDecoyB).size() == kNumberOfQueues);

  // Test that everything else is in order.
  for (unsigned i = 0; i < kNumberOfQueues; i++) {
    assert(prio_qs.Dequeue(i) == kRealValue);
    assert(prio_qs.Dequeue(i) == i);
    assert(prio_qs.Empty(i));
  }

  if (success) {
    fprintf(stderr, " Passed\n");
    return 0;
  } else
    fprintf(stderr, " Failed\n");
  return 1;
}