#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

#include "common/dstage/executor.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

const int kThreadpoolSize = 1;

DEFINE_int64(run_time, 10,
             "Length of time to run this process. Use -1 for infinite.");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  dans::Executor exec(2);
  exec.Submit({[]() { VLOG(0) << "BAM"; }});

  std::mutex wait_forever;
  wait_forever.lock();
  if (FLAGS_run_time >= 0) {
    wait_forever.unlock();
  }
  wait_forever.lock();
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_run_time));
}