#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE

#include <iostream>

#include <memory>

#include "common/dstage/job.h"
#include "common/dstage/priority.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

namespace {

using namespace dans;
using ConstJobJData = const Job<JData>;

auto kGenericData = std::make_shared<JData>(5);

}  // namespace

int main(int argc, char* argv[]) {
  static_cast<void>(argc);

  // Parse all command line flags. This MUST go before InitGoogleLogging.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  bool success = true;
  std::cerr << "test_job...";
  LOG(INFO) << "Found " << success << " cookies";

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

  assert(*original == *same_as_original);
  assert(*original != *different);

  UniqConstJobPtr<JData> decoy_a =
      std::make_unique<ConstJobJData>(kGenericData, j_fact.CreateJobId(),
                                      /*priority=*/1,
                                      /*requested_duplication=*/1);

  if (success) {
    std::cerr << " Passed\n";
    return 0;
  } else
    std::cerr << " Failed\n";
  return 1;
}