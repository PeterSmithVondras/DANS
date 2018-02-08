#include "common/dstage/dstage.h"
#include "common/dstage/dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {

using namespace dans;
using ConstJobJData = const Job<JData>;
const Priority kMaxPrio = 2;
JData kGenericData = {5};
const unsigned kGenericDuplication = 0;

class TestDispatcher : public Dispatcher<JData, int> {
 public:
  TestDispatcher(Priority max_priority)
      : Dispatcher<JData, int>(max_priority) {}

 protected:
  void DuplicateAndEnqueue(UniqConstJobPtr<JData> job_in, Priority max_prio,
                           unsigned duplication) override {
    VLOG(4) << __PRETTY_FUNCTION__ << " max_prio=" << max_prio
            << ", duplication= " << duplication;

    for (Priority prio = job_in->priority; prio <= max_prio; prio++) {
      auto duplicate_job_p = std::make_unique<const Job<int>>(
          static_cast<int>(job_in->job_data.foo), job_in->job_id, prio,
          duplication);
      _multi_q_p->Enqueue(std::move(duplicate_job_p));
    }
  }
};
}  // namespace

TEST(DStageTest, MainTest) {
  auto dispatcher = std::make_unique<TestDispatcher>(kMaxPrio);
  auto scheduler =
      std::make_unique<Scheduler<int>>(std::vector<unsigned>(kMaxPrio + 1, 2));
  auto prio_qs = std::make_unique<MultiQueue<int>>(kMaxPrio);
  DStage<JData, int> dstage(kMaxPrio, std::move(prio_qs), std::move(dispatcher),
                            std::move(scheduler));
  BaseDStage<JData>* base_dstage = &dstage;

  JobIdFactory j_fact(0);
  unsigned purged;

  auto job =
      std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                      /*priority=*/0, kGenericDuplication);
  JobId job_id = job->job_id;
  base_dstage->Dispatch(std::move(job), /*requested_duplication=*/2);
  purged = base_dstage->Purge(job_id);
  EXPECT_EQ(purged, kMaxPrio + 1);

  job = std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                        /*priority=*/1, kGenericDuplication);
  job_id = job->job_id;
  base_dstage->Dispatch(std::move(job), /*requested_duplication=*/0);
  purged = base_dstage->Purge(job_id);
  EXPECT_EQ(purged, 1);

  job = std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                        /*priority=*/2, kGenericDuplication);
  job_id = job->job_id;
  base_dstage->Dispatch(std::move(job), /*requested_duplication=*/5);
  purged = base_dstage->Purge(job_id);
  EXPECT_EQ(purged, 1);
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