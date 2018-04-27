#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

#include "common/util/callback.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

using util::Callback;

DEFINE_int64(run_time, 0,
             "Length of time to run this process. Use -1 for infinite.");

void baz() { VLOG(0) << "Baz"; }

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  auto foo = new Callback<std::unique_ptr<int>>(
      [](std::unique_ptr<int> number) { VLOG(0) << "BAM " << *number; },
      Callback<std::unique_ptr<int>>::DeleteOption::DELETE_AFTER_CALLING);

  foo->Run(std::make_unique<int>(0));
  // (*foo)(std::make_unique<int>(0));

  int x = 1;
  auto bar =
      new Callback<int>([](int number) { VLOG(0) << "BAM " << number; },
                        Callback<int>::DeleteOption::DELETE_AFTER_CALLING);
  (*bar)(x);

  Callback<> bingo(baz);

  std::function<void()> bang(std::move(bingo));
  bang();

  std::mutex wait_forever;
  wait_forever.lock();
  if (FLAGS_run_time >= 0) {
    wait_forever.unlock();
  }
  wait_forever.lock();
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_run_time));
}
