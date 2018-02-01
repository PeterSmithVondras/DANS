#include <chrono>
#include <thread>

#include "common/dstage/linux_communication_handler.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {
using dans::LinuxCommunicationHandler;
int kNumberOfSockets = 10;
}  // namespace

TEST(LinuxCommunicationHandler, CreateHandler) {
  LinuxCommunicationHandler handler;
  for (int i = 0; i < kNumberOfSockets; i++) {
    int soc = handler.CreateSocket();
    EXPECT_GE(soc, 0);
    VLOG(4) << "Socket=" << soc;
  }
  handler.Connect();
  std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  // Parse all command line flags. This MUST go before InitGoogleLogging.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  return RUN_ALL_TESTS();
}