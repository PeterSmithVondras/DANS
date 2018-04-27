#include <pthread.h>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

namespace {

using namespace dans;
using JobJData = Job<JData>;
JData kGenericData = {5};
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
  VLOG(1) << prio_qs.Describe();

  JobId decoy_a_id = j_fact.CreateJobId();
  auto decoy_a = std::make_unique<JobJData>(kGenericData, decoy_a_id,
                                            /*priority=*/0,
                                            /*duplication=*/1);
  // Purge a missing JobId returns empty list.
  CHECK_EQ(prio_qs.Purge(decoy_a->job_id), 0);

  // Purge a job with several instances.
  prio_qs.Enqueue(std::move(decoy_a));
  VLOG(1) << prio_qs.Describe();
  decoy_a = std::make_unique<JobJData>(kGenericData, decoy_a_id,
                                       /*priority=*/1,
                                       /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  VLOG(1) << prio_qs.Describe();
  decoy_a = std::make_unique<JobJData>(kGenericData, decoy_a_id,
                                       /*priority=*/2,
                                       /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  VLOG(1) << prio_qs.Describe();
  CHECK_EQ(prio_qs.Purge(decoy_a_id), kNumberOfQueues);

  // Add some JobIds with some garbage in between.
  decoy_a = std::make_unique<JobJData>(kGenericData, decoy_a_id,
                                       /*priority=*/0,
                                       /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  VLOG(1) << prio_qs.Describe();
  decoy_a = std::make_unique<JobJData>(kGenericData, decoy_a_id,
                                       /*priority=*/1,
                                       /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  VLOG(1) << prio_qs.Describe();
  decoy_a = std::make_unique<JobJData>(kGenericData, decoy_a_id,
                                       /*priority=*/2,
                                       /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_a));
  VLOG(1) << prio_qs.Describe();

  JobId target_a_id = j_fact.CreateJobId();
  auto target_a = std::make_unique<JobJData>(kGenericData, target_a_id,
                                             /*priority=*/0,
                                             /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_a));
  VLOG(1) << prio_qs.Describe();
  target_a = std::make_unique<JobJData>(kGenericData, target_a_id,
                                        /*priority=*/1,
                                        /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_a));
  VLOG(1) << prio_qs.Describe();
  target_a = std::make_unique<JobJData>(kGenericData, target_a_id,
                                        /*priority=*/2,
                                        /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_a));
  VLOG(1) << prio_qs.Describe();

  JobId decoy_b_id = j_fact.CreateJobId();
  auto decoy_b = std::make_unique<JobJData>(kGenericData, decoy_b_id,
                                            /*priority=*/0,
                                            /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_b));
  VLOG(1) << prio_qs.Describe();
  decoy_b = std::make_unique<JobJData>(kGenericData, decoy_b_id,
                                       /*priority=*/1,
                                       /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_b));
  VLOG(1) << prio_qs.Describe();
  decoy_b = std::make_unique<JobJData>(kGenericData, decoy_b_id,
                                       /*priority=*/2,
                                       /*duplication=*/1);
  prio_qs.Enqueue(std::move(decoy_b));
  VLOG(1) << prio_qs.Describe();

  JobId target_b_id = j_fact.CreateJobId();
  auto target_b = std::make_unique<JobJData>(kGenericData, target_b_id,
                                             /*priority=*/0,
                                             /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_b));
  VLOG(1) << prio_qs.Describe();
  target_b = std::make_unique<JobJData>(kGenericData, target_b_id,
                                        /*priority=*/1,
                                        /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_b));
  VLOG(1) << prio_qs.Describe();
  target_b = std::make_unique<JobJData>(kGenericData, target_b_id,
                                        /*priority=*/2,
                                        /*duplication=*/1);
  prio_qs.Enqueue(std::move(target_b));
  VLOG(1) << prio_qs.Describe();

  // Purge the garbage.
  CHECK_EQ(prio_qs.Purge(decoy_a_id), kNumberOfQueues);
  VLOG(1) << prio_qs.Describe();
  CHECK_EQ(prio_qs.Purge(decoy_b_id), kNumberOfQueues);
  VLOG(1) << prio_qs.Describe();

  // Test that everything else is in order.
  for (unsigned i = 0; i < kNumberOfQueues; i++) {
    CHECK_EQ(prio_qs.Dequeue(i)->job_id, target_a_id);
    VLOG(1) << prio_qs.Describe();
    CHECK_EQ(prio_qs.Dequeue(i)->job_id, target_b_id);
    VLOG(1) << prio_qs.Describe();
    CHECK(prio_qs.Empty(i));
    VLOG(1) << prio_qs.Describe();
  }

  prio_qs.ReleaseQueues();
  CHECK(prio_qs.Dequeue(0) == nullptr);
  VLOG(1) << prio_qs.Describe();

  LOG(INFO) << " Passed\n";
}
