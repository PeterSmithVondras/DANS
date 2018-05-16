#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <thread>

#include "common/dstage/multiqueue.h"
// #include "common/dstage/throttler.h"
#include "common/util/callback.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

using namespace dans;

DEFINE_int64(run_time, 0,
             "Length of time to run this process. Use -1 for infinite.");

void baz() { VLOG(0) << "Baz"; }

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  std::chrono::high_resolution_clock::time_point start =
      std::chrono::high_resolution_clock::now();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::chrono::high_resolution_clock::time_point end =
      std::chrono::high_resolution_clock::now();

  LOG(INFO) << "Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count();

  std::mutex wait_forever;
  wait_forever.lock();
  if (FLAGS_run_time >= 0) {
    wait_forever.unlock();
  }
  wait_forever.lock();
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_run_time));
}
