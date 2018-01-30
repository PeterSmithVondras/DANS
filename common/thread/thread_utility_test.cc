#include "common/thread/thread_utility.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {
using threadutility::ThreadUtility;
}  // namespace

TEST(PriorityThreadTest, CreateThread) {
  std::thread thread([](int foo) {}, /*foo=*/5);
  ThreadUtility::SetPriority(thread, SCHED_RR, 2);
  EXPECT_EQ(true, true);
  thread.join();
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