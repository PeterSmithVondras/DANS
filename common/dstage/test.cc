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

template <typename T>
class Outer {
 public:
  class Inner {
   public:
    Inner() {}
  };

  Outer() {}
};

template <typename T>
using UniqOuterPtr = std::unique_ptr<Outer<T>>;

template <typename T>
using UniqInnerPtr = std::unique_ptr<typename Outer<T>::Inner>;

DEFINE_int64(run_time, 0,
             "Length of time to run this process. Use -1 for infinite.");

void baz() { VLOG(0) << "Baz"; }

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  // MultiQueue<int> q(1);
  // Throttler<int> t(&q, {5, 1});
  // auto foo = t.Dequeue(0);

  // auto foo = std::make_unique<Outer<int>::Inner>();

  char character = 'f';
  std::srand(std::time(nullptr));
  int random_number = std::rand() % 2;
  std::array<std::atomic<bool>, 1> condition = {false};
  condition = true;
  if (condition[0].load() && random_number == 1) {
    character = 't';
  }

  LOG(INFO) << "Character was: " << character
            << " random_number: " << random_number;

  std::mutex wait_forever;
  wait_forever.lock();
  if (FLAGS_run_time >= 0) {
    wait_forever.unlock();
  }
  wait_forever.lock();
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_run_time));
}
