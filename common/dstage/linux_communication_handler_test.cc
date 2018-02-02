#include <sys/socket.h>
#include <sys/types.h>

#include <chrono>
#include <cstring>
#include <functional>
#include <thread>

#include "common/dstage/linux_communication_handler.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace {
using dans::LinuxCommunicationHandler;
using CallBack2 = LinuxCommunicationHandler::CallBack2;
int kNumberOfSockets = 10;
}  // namespace

void MonitorCallback(LinuxCommunicationHandler* handler, int soc,
                     LinuxCommunicationHandler::ReadyFor ready_for) {
  if (ready_for.in) {
    char buf[15];
    read(soc, buf, 15);
    LOG(INFO) << buf;
    handler->Close(soc);
  }
}

void ConnectCallback(LinuxCommunicationHandler* handler, int soc,
                     LinuxCommunicationHandler::ReadyFor ready_for) {
  char buf[] =
      "GET / HTTP/1.1\n"
      "Host: www.google.com\n"
      "User-Agent: curl/7.54.0\n"
      "Accept: */*\n"
      "\n";

  int ret = send(soc, buf, std::strlen(buf), /*flags=*/0);
  PLOG_IF(ERROR, ret != static_cast<int>(std::strlen(buf)))
      << "Failed to send: socket=" << soc << "\n"
      << buf;

  CallBack2 done(std::bind(MonitorCallback, handler, std::placeholders::_1,
                           std::placeholders::_2));
  handler->Monitor(soc,
                   LinuxCommunicationHandler::ReadyFor{/*in=*/true,
                                                       /*out=*/false},
                   done);
}

TEST(LinuxCommunicationHandler, CreateHandler) {
  LinuxCommunicationHandler handler;
  for (int i = 0; i < kNumberOfSockets; i++) {
    int soc = handler.CreateSocket();
    EXPECT_GE(soc, 0);
    VLOG(4) << "Socket=" << soc;
  }

  CallBack2 done(std::bind(ConnectCallback, &handler, std::placeholders::_1,
                           std::placeholders::_2));
  handler.Connect("172.217.10.36", "80", done);

  std::this_thread::sleep_for(std::chrono::milliseconds(250));
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