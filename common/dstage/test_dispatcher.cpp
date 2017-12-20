#include <mutex>

#include "common/dstage/dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"

#include <cassert>
#include <cstdlib>  // EXIT_SUCCESS and EXIT_FAILURE
namespace {
unsigned kMaxDuplication = 3;

using namespace duplicate_aware_scheduling;

}  // namespace

int main() {
  bool success = true;
  fprintf(stderr, "test_dispatcher...\n");

  JobIdFactory j_factory(0);
  Job<unsigned> job(0, j_factory.CreateJobId(), 0, 3);

  MultiQueue mq(kMaxDuplication);
  Dispatcher<unsigned> disp(kMaxDuplication);
  std::mutex q_mutex;

  disp.LinkMultiQ(&mq, &q_mutex);

  if (success) {
    printf(" Passed\n");
    return 0;
  } else
    return 1;
}