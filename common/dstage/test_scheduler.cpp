#include <memory>
#include <mutex>
#include <utility>

#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

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
  fprintf(stderr, "test_scheduler...");

  Scheduler<JData> scheduler(kMaxPrio);
  MultiQueue<JData> prio_qs(kMaxPrio);
  scheduler.LinkMultiQ(&prio_qs);
  scheduler.Run();

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

  if (success) {
    fprintf(stderr, " Passed\n");
    return 0;
  } else
    fprintf(stderr, " Failed\n");
  return 1;
}
