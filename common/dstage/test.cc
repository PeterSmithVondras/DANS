#include <functional>

#include "gflags/gflags.h"
#include "glog/logging.h"

void foo(int a, int b, int c) {
	CHECK(a + b + c > 10);
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  using DynamicallyAllocatedCallback = std::function<void(uint32_t)>;

  int a = 10;
  int b = 20;

  auto cb_p = new DynamicallyAllocatedCallback(
      std::bind(foo, a, b,
                std::placeholders::_1));

  (*cb_p)(9);
  delete cb_p;
}