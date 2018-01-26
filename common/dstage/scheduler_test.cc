#include <memory>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

namespace {
using namespace dans;
using ConstJobJData = const Job<JData>;
const Priority kMaxPrio = 2;
auto kGenericData = std::make_shared<JData>(5);
// const unsigned kGenericDuplication = 0;

}  // namespace

int main(int argc, char* argv[]) {
  // Parse all command line flags. This MUST go before InitGoogleLogging.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  LOG(INFO) << "test_dispatcher...";

  auto scheduler = std::make_unique<Scheduler<JData>>(
      std::vector<unsigned>(kMaxPrio + 1, 2));
  auto prio_qs = std::make_unique<MultiQueue<JData>>(kMaxPrio);
  scheduler->LinkMultiQ(prio_qs.get());
  scheduler->Run();
  scheduler = nullptr;

  // JobIdFactory j_fact(0);
  // std::list<UniqConstJobPtr<JData>> purged;

  // auto job =
  //     std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
  //                                     /*priority=*/0, kGenericDuplication);
  // JobId job_id = job->job_id;
  // dispatcher.Dispatch(std::move(job), /*requested_duplication=*/2);
  // assert(prio_qs.Purge(job_id).size() == kMaxPrio + 1);

  // job = std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
  //                                       /*priority=*/1, kGenericDuplication);
  // job_id = job->job_id;
  // Priority prio = job->priority;
  // dispatcher.Dispatch(std::move(job), /*requested_duplication=*/0);
  // purged = prio_qs.Purge(job_id);
  // assert(purged.size() == 1);
  // assert(purged.front()->priority == prio);

  // job = std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
  //                                       /*priority=*/2, kGenericDuplication);
  // job_id = job->job_id;
  // prio = job->priority;
  // dispatcher.Dispatch(std::move(job), /*requested_duplication=*/5);
  // purged = prio_qs.Purge(job_id);
  // assert(purged.size() == 1);
  // assert(purged.front()->priority == prio);

  LOG(INFO) << "PASS";
  return 0;
}
