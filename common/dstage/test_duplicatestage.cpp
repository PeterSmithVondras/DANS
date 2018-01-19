#include <mutex>

#include "common/dstage/dispatcher.h"
#include "common/dstage/duplicatestage.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE

namespace {

using namespace dans;
using ConstJobJData = const Job<JData>;
const Priority kMaxPrio = 2;
auto kGenericData = std::make_shared<JData>(5);
const unsigned kGenericDuplication = 0;

}  // namespace

int main() {
  bool success = true;
  fprintf(stderr, "test_duplicatestage...");

  auto dispatcher = std::make_unique<Dispatcher<JData>>(kMaxPrio);
  DuplicateStage<JData> dstage(kMaxPrio, std::move(dispatcher));

  JobIdFactory j_fact(0);
  std::list<UniqConstJobPtr<JData>> purged;

  // Job<JData> job(kGenericData, /*job_id=*/j_fact.CreateJobId(),
  //                /*priority=*/0, /*requested_duplication=*/2);
  // dstage.Dispatch(job);
  // assert(prio_qs.Purge(job.job_id).size() == kMaxPrio + 1);

  // job = Job<JData>(kGenericData, /*job_id=*/j_fact.CreateJobId(),
  //                  /*priority=*/1, /*requested_duplication=*/0);
  // dstage.Dispatch(job);
  // purged = prio_qs.Purge(job.job_id);
  // assert(purged.size() == 1);
  // assert(purged.front() == job.priority);

  // job = Job<JData>(kGenericData, /*job_id=*/j_fact.CreateJobId(),
  //                  /*priority=*/2, /*requested_duplication=*/5);
  // dstage.Dispatch(job);
  // purged = prio_qs.Purge(job.job_id);
  // assert(purged.size() == 1);
  // assert(purged.front() == job.priority);

  if (success) {
    fprintf(stderr, " Passed\n");
    return 0;
  } else
    fprintf(stderr, " Failed\n");
  return 1;
}