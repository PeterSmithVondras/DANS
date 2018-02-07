#include <memory>
#include <string>

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

  LOG(INFO) << "PASS";
  return 0;
}
