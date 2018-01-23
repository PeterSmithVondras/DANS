#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE

#include <iostream>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

namespace {

using namespace dans;
using ConstJobJData = const Job<JData>;
auto kGenericData = std::make_shared<JData>(5);
unsigned kNumberOfQueues = 3;

}  // namespace

int main() {
  bool success = true;
  std::cerr << "test_multiqueue... ";

  JobIdFactory j_fact(0);
  MultiQueue<JData> prio_qs(kNumberOfQueues);

  JobId decoy_a_id = j_fact.CreateJobId();
  auto decoy_a = std::make_unique<ConstJobJData>(kGenericData, decoy_a_id,
                                                 /*priority=*/0,
                                                 /*duplication=*/1);
  // Purge a missing JobId returns empty list.
  assert(prio_qs.Purge(decoy_a->job_id).empty());

  // Purge a job with several instances.
  prio_qs.Enqueue(std::move(decoy_a));
  decoy_a = std::make_unique<ConstJobJData>(kGenericData, decoy_a_id,
                                            /*priority=*/1,
                                            /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  decoy_a = std::make_unique<ConstJobJData>(kGenericData, decoy_a_id,
                                            /*priority=*/2,
                                            /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  assert(prio_qs.Purge(decoy_a_id).size() == kNumberOfQueues);

  // Add some JobIds with some garbage in between.
  decoy_a = std::make_unique<ConstJobJData>(kGenericData, decoy_a_id,
                                            /*priority=*/0,
                                            /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  decoy_a = std::make_unique<ConstJobJData>(kGenericData, decoy_a_id,
                                            /*priority=*/1,
                                            /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  decoy_a = std::make_unique<ConstJobJData>(kGenericData, decoy_a_id,
                                            /*priority=*/2,
                                            /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));

  JobId target_a_id = j_fact.CreateJobId();
  auto target_a = std::make_unique<ConstJobJData>(kGenericData, target_a_id,
                                                  /*priority=*/0,
                                                  /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_a));
  target_a = std::make_unique<ConstJobJData>(kGenericData, target_a_id,
                                             /*priority=*/1,
                                             /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_a));
  target_a = std::make_unique<ConstJobJData>(kGenericData, target_a_id,
                                             /*priority=*/2,
                                             /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_a));

  JobId decoy_b_id = j_fact.CreateJobId();
  auto decoy_b = std::make_unique<ConstJobJData>(kGenericData, decoy_b_id,
                                                 /*priority=*/0,
                                                 /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_b));
  decoy_b = std::make_unique<ConstJobJData>(kGenericData, decoy_b_id,
                                            /*priority=*/1,
                                            /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_b));
  decoy_b = std::make_unique<ConstJobJData>(kGenericData, decoy_b_id,
                                            /*priority=*/2,
                                            /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_b));

  JobId target_b_id = j_fact.CreateJobId();
  auto target_b = std::make_unique<ConstJobJData>(kGenericData, target_b_id,
                                                  /*priority=*/0,
                                                  /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_b));
  target_b = std::make_unique<ConstJobJData>(kGenericData, target_b_id,
                                             /*priority=*/1,
                                             /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_b));
  target_b = std::make_unique<ConstJobJData>(kGenericData, target_b_id,
                                             /*priority=*/2,
                                             /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_b));

  // Purge the garbage.
  assert(prio_qs.Purge(decoy_a_id).size() == kNumberOfQueues);
  assert(prio_qs.Purge(decoy_b_id).size() == kNumberOfQueues);

  // Test that everything else is in order.
  for (unsigned i = 0; i < kNumberOfQueues; i++) {
    assert(prio_qs.Dequeue(i)->job_id == target_a_id);
    assert(prio_qs.Dequeue(i)->job_id == target_b_id);
    assert(prio_qs.Empty(i));
  }

  prio_qs.ReleaseQueues();
  assert(prio_qs.Dequeue(0) == nullptr);

  if (success) {
    std::cerr << " Passed\n";
    return 0;
  }

  std::cerr << " Failed\n";
  return 1;
}
