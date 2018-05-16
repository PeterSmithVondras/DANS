#include <chrono>
#include <csignal>
#include <functional>

#include "common/dstage/throttler.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_int64(run_time, -1,
             "Length of time to run this process. Use -1 for infinite.");

namespace {
const unsigned kMaxPrio = 1;
const unsigned kThreadsPerPrio = 1;
const unsigned kGetRequestsTotal = 2;
const unsigned kPurgeThreadPoolThreads = 2;
const unsigned kHighPriority = 0;
const unsigned kLowPriority = 1;
}  // namespace

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  LOG(INFO) << "Hello World";

  std::mutex wait_forever;
  wait_forever.lock();
  if (FLAGS_run_time >= 0) {
    wait_forever.unlock();
  }
  wait_forever.lock();
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_run_time));
}
