#include <chrono>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>

#include "common/dstage/client_connect_handler.h"
#include "common/dstage/client_request_handler.h"
#include "common/dstage/client_response_handler.h"
#include "common/dstage/dstage.h"
#include "common/dstage/job_types.h"
#include "common/dstage/linux_communication_handler.h"
#include "common/dstage/scheduler.h"
#include "common/dstage/synchronization.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

DEFINE_bool(set_thread_priority, false,
            "Set thread priority with Linux OS, "
            "which requires running with `sudo`.");

namespace {
using namespace dans;
const unsigned kMaxPrio = 1;
const unsigned kThreadsPerPrio = 1;
const unsigned kGetRequestsTotal = 2;

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

    _response_dstage->RegisterOriginDStage(_connect_dstage.get());
  }

  std::timed_mutex _complete_lock;
  LinuxCommunicationHandler _comm_handler;
  std::unique_ptr<ResponseDStage> _response_dstage;
  std::unique_ptr<BaseDStage<RequestData>> _request_dstage;
  std::unique_ptr<BaseDStage<ConnectData>> _connect_dstage;
};

TEST_F(FileClientDstageChainTest, CreateConnect) {
  Counter counter(0);
  std::timed_mutex complete_lock;
  complete_lock.lock();
  std::vector<std::unique_ptr<std::vector<char>>> files(kGetRequestsTotal);
  auto response = std::make_shared<
      std::function<void(unsigned, int, std::unique_ptr<std::vector<char>>)>>(
      [&counter, &complete_lock, &files](
          unsigned priority, int object_id,
          std::unique_ptr<std::vector<char>> object) {
        LOG(INFO) << "Received file=" << object_id;
        files[object_id] = std::move(object);

        counter.Increment();
        if (counter.Count() == kGetRequestsTotal) {
          complete_lock.unlock();
        }
      });

  ConnectData connect_data = {
      {"192.168.137.127", "192.168.137.127"}, /*file=*/0, response};
  UniqJobPtr<ConnectData> job;
  for (unsigned i = 0; i < kGetRequestsTotal; i++) {
    connect_data.object_id = static_cast<int>(i);
    job = std::make_unique<Job<ConnectData>>(connect_data,
                                             /*job_id=*/i,
                                             /*priority=*/0,
                                             /*duplication*/ 0);
    _connect_dstage->Dispatch(std::move(job), /*requested_duplication=*/1);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(10));  // slow server.... :)
  }

  EXPECT_TRUE(complete_lock.try_lock_for(std::chrono::milliseconds(10000)))
      << "Did not get all responses. Check internet connection.";
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(kGetRequestsTotal, counter.Count());

  // Write files to disk.
  std::string t_str = "T";
  int i = 0;
  for (const auto &object : files) {
    std::ofstream output_file(t_str + std::to_string(i));
    for (const auto &e : *object) output_file << e;
    output_file.close();
    i++;
  }
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