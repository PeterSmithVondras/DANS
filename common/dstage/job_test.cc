#include <memory>

#include "common/dstage/job.h"
#include "common/dstage/priority.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

namespace {
using namespace dans;
using ConstJobJData = const Job<JData>;
JData kGenericData = {5};
}  // namespace

int main(int argc, char* argv[]) {
  // Parse all command line flags. This MUST go before InitGoogleLogging.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  LOG(INFO) << "test_job...";

  JobIdFactory j_fact(0);

  auto original =
      std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                      /*priority=*/1,
                                      /*requested_duplication=*/1);

  auto same_as_original =
      std::make_unique<ConstJobJData>(kGenericData, original->job_id,
                                      /*priority=*/2,
                                      /*requested_duplication=*/2);

  auto different =
      std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                      /*priority=*/3,
                                      /*requested_duplication=*/3);

  CHECK(*original == *same_as_original);
  CHECK(*original != *different);

  UniqConstJobPtr<JData> decoy_a =
      std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                      /*priority=*/1,
                                      /*requested_duplication=*/1);

  LOG(INFO) << "PASS";
  return 0;
}