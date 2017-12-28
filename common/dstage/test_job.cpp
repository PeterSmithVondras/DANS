#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE

#include <iostream>

#include <memory>

#include "common/dstage/job.h"
#include "common/dstage/priority.h"

namespace {

using namespace duplicate_aware_scheduling;
auto kGenericData = std::make_shared<const JData>(5);

}  // namespace

int main() {
  bool success = true;
  std::cerr << "test_job...";

  JobIdFactory j_fact(0);

  Job<JData> original(kGenericData, /*job_id=*/j_fact.CreateJobId(),
                      /*priority=*/1, /*requested_duplication=*/1);

  Job<JData> same_as_original(kGenericData, /*job_id=*/original.job_id,
                              /*priority=*/2, /*requested_duplication=*/2);

  Job<JData> different(kGenericData, /*job_id=*/j_fact.CreateJobId(),
                       /*priority=*/3, /*requested_duplication=*/3);

  assert(original == same_as_original);
  assert(original != different);

  if (success) {
    std::cerr << " Passed\n";
    return 0;
  } else
    std::cerr << " Failed\n";
  return 1;
}