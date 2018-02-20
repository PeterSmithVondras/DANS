#include <chrono>
#include <functional>
#include <memory>
#include <mutex>

#include "common/dstage/client_connect_handler.h"
#include "common/dstage/client_request_handler.h"
#include "common/dstage/client_response_handler.h"
#include "common/dstage/dstage.h"
#include "common/dstage/linux_communication_handler.h"
#include "common/dstage/scheduler.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

DEFINE_bool(set_thread_priority, false,
            "Set thread priority with Linux OS, "
            "which requires running with `sudo`.");

namespace {
using namespace dans;
const unsigned kMaxPrio = 1;
const unsigned kThreadsPerPrio = 2;

}  // namespace

class FileClientDstageChainTest : public testing::Test {
 protected:
  virtual void SetUp() {
    _complete_lock.lock();

    _response_dstage = std::make_unique<ResponseDStage>(
        std::vector<unsigned>(kMaxPrio + 1, kThreadsPerPrio),
        FLAGS_set_thread_priority, &_comm_handler);

    _request_dstage = std::make_unique<RequestDStage>(
        std::vector<unsigned>(kMaxPrio + 1, kThreadsPerPrio),
        FLAGS_set_thread_priority, &_comm_handler, _response_dstage.get());

    _connect_dstage = std::make_unique<ConnectDStage>(
        std::vector<unsigned>(kMaxPrio + 1, kThreadsPerPrio),
        FLAGS_set_thread_priority, &_comm_handler, _request_dstage.get());
  }

  std::timed_mutex _complete_lock;
  LinuxCommunicationHandler _comm_handler;
  std::unique_ptr<BaseDStage<RequestData>> _response_dstage;
  std::unique_ptr<BaseDStage<RequestData>> _request_dstage;
  std::unique_ptr<BaseDStage<ConnectData>> _connect_dstage;
};

TEST_F(FileClientDstageChainTest, CreateConnect) {
  ConnectData connect_data = {
      {"172.217.10.36", "172.217.10.36"},
      {"80", "80"},
      std::make_shared<std::function<void(int)>>([](int foo) {})};
  UniqConstJobPtr<ConnectData> job;
  for (unsigned i = 0; i < 100; i++) {
    job = std::make_unique<ConstJob<ConnectData>>(connect_data,
                                                  /*job_id=*/i,
                                                  /*priority=*/0,
                                                  /*duplication*/ 0);
    _connect_dstage->Dispatch(std::move(job), /*requested_duplication=*/1);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

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