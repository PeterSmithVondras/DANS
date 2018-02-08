#include <functional>
#include <memory>
#include <mutex>

#include "common/dstage/client_request_handler.h"
#include "common/dstage/dstage.h"
#include "common/dstage/scheduler.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

DEFINE_bool(set_thread_priority, false,
            "Set thread priority with Linux OS, "
            "which requires running with `sudo`.");

namespace {
using namespace dans;
// using CallBack2 = LinuxCommunicationHandler::CallBack2;
// int kNumberOfSockets = 10;
// int kReadSize = 15;
unsigned kMaxPrio = 1;
}  // namespace

class FileClientDstageChainTest : public testing::Test {
 protected:
  virtual void SetUp() {
    _complete_lock.lock();
    RequestDispatcher disp();
    auto dispatcher = std::make_unique<RequestDispatcher>(kMaxPrio);
    auto scheduler = std::make_unique<RequestScheduler>(
        std::vector<unsigned>(kMaxPrio + 1, 2), FLAGS_set_thread_priority);
    auto multiq = std::make_unique<MultiQueue<ReqDataInternal>>(kMaxPrio);

    request_dstage = std::make_unique<DStage<ReqData, ReqDataInternal>>(
        kMaxPrio, std::move(multiq), std::move(dispatcher),
        std::move(scheduler));
  }

  std::timed_mutex _complete_lock;
  std::unique_ptr<BaseDStage<ReqData>> request_dstage;
};

TEST_F(FileClientDstageChainTest, CreateRequest) { EXPECT_TRUE(true); }

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  // Parse all command line flags. This MUST go before InitGoogleLogging.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Provides a failure signal handler.
  google::InstallFailureSignalHandler();

  return RUN_ALL_TESTS();
}