#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE

#include <iostream>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace {

using namespace duplicate_aware_scheduling;

const JData kGenericData{5};
unsigned kNumberOfQueues = 3;

}  // namespace

int main() {
  bool success = true;
  std::cerr << "test_multiqueue... ";

  JobIdFactory j_fact(0);

  auto decoy_a = std::make_shared<const Job<JData>>(
      kGenericData, /*job_id=*/j_fact.CreateJobId(),
      /*priority=*/1, /*requested_duplication=*/1);
  auto decoy_b = std::make_shared<const Job<JData>>(
      kGenericData, /*job_id=*/j_fact.CreateJobId(),
      /*priority=*/1, /*requested_duplication=*/1);
  auto target_a = std::make_shared<const Job<JData>>(
      kGenericData, /*job_id=*/j_fact.CreateJobId(),
      /*priority=*/1, /*requested_duplication=*/1);
  auto target_b = std::make_shared<const Job<JData>>(
      kGenericData, /*job_id=*/j_fact.CreateJobId(),
      /*priority=*/1, /*requested_duplication=*/1);

  MultiQueue<JobId, std::shared_ptr<const Job<JData>>> prio_qs(kNumberOfQueues);

  // // Purge a missing JobId returns empty list.
  // assert(prio_qs.Purge(kDecoyA).empty());

  // Purge a job with several instances.
  // prio_qs.Enqueue(std::make_pair(decoy_a->job_id, decoy_a), 0);
  prio_qs.Enqueue({decoy_a->job_id, decoy_a}, 0);
  prio_qs.Enqueue({decoy_a->job_id, decoy_a}, 1);
  prio_qs.Enqueue({decoy_a->job_id, decoy_a}, 2);
  assert(prio_qs.Purge(decoy_a->job_id).size() == kNumberOfQueues);

  // // Add some JobIds with some garbage in between.
  prio_qs.Enqueue({decoy_a->job_id, decoy_a}, 0);
  prio_qs.Enqueue({decoy_a->job_id, decoy_a}, 1);
  prio_qs.Enqueue({decoy_a->job_id, decoy_a}, 2);

  prio_qs.Enqueue({target_a->job_id, target_a}, 0);
  prio_qs.Enqueue({target_a->job_id, target_a}, 1);
  prio_qs.Enqueue({target_a->job_id, target_a}, 2);

  prio_qs.Enqueue({decoy_b->job_id, decoy_b}, 0);
  prio_qs.Enqueue({decoy_b->job_id, decoy_b}, 1);
  prio_qs.Enqueue({decoy_b->job_id, decoy_b}, 2);

  prio_qs.Enqueue({target_b->job_id, target_b}, 0);
  prio_qs.Enqueue({target_b->job_id, target_b}, 1);
  prio_qs.Enqueue({target_b->job_id, target_b}, 2);

  // // Purge the garbage.
  assert(prio_qs.Purge(decoy_a->job_id).size() == kNumberOfQueues);
  assert(prio_qs.Purge(decoy_b->job_id).size() == kNumberOfQueues);

  // Test that everything else is in order.
  for (unsigned i = 0; i < kNumberOfQueues; i++) {
    assert(*prio_qs.Dequeue(i) == *target_a);
    assert(*prio_qs.Dequeue(i) == *target_b);
    assert(prio_qs.Empty(i));
  }

  if (success) {
    std::cerr << " Passed\n";
    return 0;
  } else
    std::cerr << " Failed\n";
  return 1;
}
