#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE

#include <iostream>

#include "common/dstage/job.h"
#include "common/dstage/priority.h"

namespace {

using namespace duplicate_aware_scheduling;

}  // namespace

int main() {
  bool success = true;
  std::cerr << "test_job...";

  JobIdFactory j_fact(0);

  Job<JData> job(/*job_data=*/0, /*job_id=*/j_fact.CreateJobId(),
                 /*priority=*/0, /*requested_duplication=*/2);

  job = Job<JData>(/*job_data=*/0, /*job_id=*/j_fact.CreateJobId(),
                   /*priority=*/1, /*requested_duplication=*/0);

  job = Job<JData>(/*job_data=*/0, /*job_id=*/j_fact.CreateJobId(),
                   /*priority=*/2, /*requested_duplication=*/5);

  if (success) {
    std::cerr << " Passed\n";
    return 0;
  } else
    std::cerr << " Failed\n";
  return 1;
}