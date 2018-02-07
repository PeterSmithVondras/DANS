#include <memory>

#include "common/dstage/dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {
using namespace dans;
using ConstJobJData = const Job<JData>;
const Priority kMaxPrio = 2;
auto kGenericData = std::make_shared<JData>(5);
const unsigned kGenericDuplication = 0;

class TestDispatcher : public Dispatcher<JData, JData> {
 public:
  TestDispatcher(Priority max_priority)
      : Dispatcher<JData, JData>(max_priority) {}

 protected:
  UniqConstJobPtr<JData> DuplicateAndConvert(const Job<JData>* job_in,
                                             Priority prio,
                                             unsigned duplication) override {
    VLOG(4) << __PRETTY_FUNCTION__ << " prio=" << prio
            << ", duplication= " << duplication;
    return std::make_unique<const Job<JData>>(job_in->job_data, job_in->job_id,
                                              prio, duplication);
  }
};
}  // namespace

TEST(DispatcherTest, MainTest) {
  auto dispatcher = std::make_unique<TestDispatcher>(kMaxPrio);
  auto prio_qs = std::make_unique<MultiQueue<JData>>(kMaxPrio);
  dispatcher->LinkMultiQ(prio_qs.get());

  JobIdFactory j_fact(0);
  std::list<UniqConstJobPtr<JData>> purged;

  auto job =
      std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                      /*priority=*/0, kGenericDuplication);
  JobId job_id = job->job_id;
  dispatcher->Dispatch(std::move(job), /*requested_duplication=*/2);
  EXPECT_EQ(prio_qs->Purge(job_id).size(), kMaxPrio + 1);

  job = std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                        /*priority=*/1, kGenericDuplication);
  job_id = job->job_id;
  Priority prio = job->priority;
  dispatcher->Dispatch(std::move(job), /*requested_duplication=*/0);
  purged = prio_qs->Purge(job_id);
  EXPECT_EQ(purged.size(), 1);
  EXPECT_EQ(purged.front()->priority, prio);

  job = std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                        /*priority=*/2, kGenericDuplication);
  job_id = job->job_id;
  prio = job->priority;
  dispatcher->Dispatch(std::move(job), /*requested_duplication=*/5);
  purged = prio_qs->Purge(job_id);
  EXPECT_EQ(purged.size(), 1);
  EXPECT_EQ(purged.front()->priority, prio);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  // Parse all command line flags. This MUST go before InitGoogleLogging.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  return RUN_ALL_TESTS();
}