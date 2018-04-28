#include <sys/socket.h>
#include <sys/types.h>

#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "common/dstage/linux_communication_handler.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {
using dans::LinuxCommunicationHandler;
using ReadyFor = dans::CommunicationHandlerInterface::ReadyFor;
using CallBack2 = dans::CommunicationHandlerInterface::CallBack2;
int kNumberOfSockets = 10;
int kReadSize = 15;
}  // namespace

class LinuxCommunicationHandlerTest : public testing::Test {
 protected:
  virtual void SetUp() { _complete_lock.lock(); }

  void ReadCallback(int soc, ReadyFor ready_for) {
    if (ready_for.in) {
      EXPECT_TRUE(ready_for.in);
      char buf[kReadSize + 1];
      EXPECT_EQ(read(soc, buf, kReadSize), kReadSize);
      buf[kReadSize] = '\0';
      EXPECT_STREQ(buf, "HTTP/1.1 200 OK");
      VLOG(1) << buf;
      _handler.Close(soc);
      // Signal that we have completed the chain of events.
      _complete_lock.unlock();
    }
  }

  void SendCallback(int soc, ReadyFor ready_for) {
    EXPECT_TRUE(ready_for.out) << "Failed to create TCP connection.";
    EXPECT_FALSE(ready_for.in) << "Failed to create TCP connection.";
    char buf[] =
        "GET / HTTP/1.1\n"
        "Host: www.google.com\n"
        "User-Agent: curl/7.54.0\n"
        "Accept: */*\n"
        "\n";

    int ret = send(soc, buf, std::strlen(buf), /*flags=*/0);
    EXPECT_EQ(ret, static_cast<int>(std::strlen(buf)))
        << "Failed to send: socket=" << soc << "\n"
        << "Tried to send: " << buf;

    CallBack2 done(std::bind(&LinuxCommunicationHandlerTest::ReadCallback, this,
                             std::placeholders::_1, std::placeholders::_2));
    _handler.Monitor(soc,
                     ReadyFor{/*in=*/true,
                              /*out=*/false},
                     done);
  }

  // This function is used for a test, but must be written inside of the
  // LinuxCommunicationHandlerTest class because of google magic, where it will
  // not allow passing of non-static member functions from within the TEST_F.
  void Get() {
    CallBack2 done(std::bind(&LinuxCommunicationHandlerTest::SendCallback, this,
                             std::placeholders::_1, std::placeholders::_2));
    _handler.Connect("172.217.10.36", "80", done);
    EXPECT_TRUE(_complete_lock.try_lock_for(std::chrono::milliseconds(250)))
        << "ReadCallback was never called which means that we never received"
        << "an HTTP response. Check internet connection.";
  }

  std::timed_mutex _complete_lock;
  LinuxCommunicationHandler _handler;
};

TEST_F(LinuxCommunicationHandlerTest, CreateSocket) {
  for (int i = 0; i < kNumberOfSockets; i++) {
    int soc = _handler.CreateSocket();
    EXPECT_GE(soc, 0);
    VLOG(4) << "Socket=" << soc;
  }
}

TEST_F(LinuxCommunicationHandlerTest, GetGoogle) { Get(); }

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