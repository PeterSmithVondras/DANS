#include <pthread.h>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

namespace {

using namespace dans;
using ConstJobJData = const Job<JData>;
auto kGenericData = std::make_shared<JData>(5);
unsigned kNumberOfQueues = 3;

}  // namespace

int main(int argc, char* argv[]) {
  // Parse all command line flags. This MUST go before InitGoogleLogging.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  LOG(INFO) << "test_multiqueue... ";

  JobIdFactory j_fact(0);
  MultiQueue<JData> prio_qs(kNumberOfQueues);

  JobId decoy_a_id = j_fact.CreateJobId();
  auto decoy_a = std::make_unique<ConstJobJData>(kGenericData, decoy_a_id,
                                                 /*priority=*/0,
                                                 /*duplication=*/1);
  // Purge a missing JobId returns empty list.
  CHECK(prio_qs.Purge(decoy_a->job_id).empty());

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
  CHECK_EQ(prio_qs.Purge(decoy_a_id).size(), kNumberOfQueues);

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
  CHECK_EQ(prio_qs.Purge(decoy_a_id).size(), kNumberOfQueues);
  CHECK_EQ(prio_qs.Purge(decoy_b_id).size(), kNumberOfQueues);

  // Test that everything else is in order.
  for (unsigned i = 0; i < kNumberOfQueues; i++) {
    CHECK_EQ(prio_qs.Dequeue(i)->job_id, target_a_id);
    CHECK_EQ(prio_qs.Dequeue(i)->job_id, target_b_id);
    CHECK(prio_qs.Empty(i));
  }

  prio_qs.ReleaseQueues();
  CHECK(prio_qs.Dequeue(0) == nullptr);

  LOG(INFO) << " Passed\n";
}
