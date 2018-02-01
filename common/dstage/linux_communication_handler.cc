#include "common/dstage/linux_communication_handler.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cerrno>
#include <cstring>

#include <functional>
#include <memory>

#include "gflags/gflags.h"
#include "glog/logging.h"

namespace {
// Currently have no flags to pass to epoll_create.
int kFlags = 0;
int kMaxEvents = 64;
int kTimeoutInMilliseconds = 250;
}  // namespace

namespace dans {

LinuxCommunicationHandler::LinuxCommunicationHandler()
    : _events(kMaxEvents), _destructing(false) {
  VLOG(4) << __PRETTY_FUNCTION__;
  _epoll_fd = epoll_create1(kFlags);
  PLOG_IF(ERROR, _epoll_fd < 0)
      << "LinuxCommunicationHandler failed to create epoll filedescriptor.";
  _socket_handler =
      std::thread(&LinuxCommunicationHandler::MonitorSockets, this);
}

LinuxCommunicationHandler::~LinuxCommunicationHandler() {
  VLOG(4) << __PRETTY_FUNCTION__;
  {
    std::unique_lock<std::shared_timed_mutex> lock(_destructing_lock);
    _destructing = true;
  }
  _socket_handler.join();

  PLOG_IF(ERROR, close(_epoll_fd) != 0)
      << "Failure to close epoll file descriptor";
}

int LinuxCommunicationHandler::CreateSocket() {
  VLOG(4) << __PRETTY_FUNCTION__;
  int soc = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  PLOG_IF(ERROR, soc == -1) << "Failed to create socket.";
  return soc;
}

void LinuxCommunicationHandler::Connect() {
  VLOG(4) << __PRETTY_FUNCTION__;
  int soc = CreateSocket();

  // Hard coded as google.
  const char* ip = "172.217.10.36";
  const unsigned short port = atoi("80");
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_aton(ip, &addr.sin_addr);

  bool connected = true;
  int ret =
      connect(soc, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in));
  if ((ret != 0) && (errno == EINPROGRESS)) {
    // Connection not immediate so we will have to use epoll.
    connected = false;
  } else if (ret != 0) {
    PLOG(ERROR) << "Failed to connect socket with initial call to connect().";
  } else {
    LOG(WARNING) << "Actually connected without EINPROGRESS!";
  }

  struct epoll_event event;
  // Need to wait on socket for ability to connect.
  event.events = EPOLLOUT;

  // Setting user data in the event structure to be a callback with relevant
  // data.
  // event.data.ptr = new DynamicallyAllocatedCallback(std::bind(SocketReady,
  //                                                             soc,
  //                                                             connected));
  event.data.ptr = new DynamicallyAllocatedCallback(
      std::bind(&LinuxCommunicationHandler::SocketReady, this, soc, connected));
  LOG(INFO) << "Created socket for connect: socket=" << soc;
  ret = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, soc, &event);
  PLOG_IF(ERROR, ret != 0) << "Failed to add socket to epoll set: socket="
                           << soc;
}

void LinuxCommunicationHandler::MonitorSockets() {
  VLOG(4) << __PRETTY_FUNCTION__;
  while (true) {
    // Waiting for available socket.
    int returned_events_count = epoll_wait(
        _epoll_fd, _events.data(), _events.size(), kTimeoutInMilliseconds);

    // Handle every returned socket.
    for (int i = 0; i < returned_events_count; i++) {
      LOG(INFO) << "Got a Socket";
      if (!_events[i].events) continue;
      auto cb_p =
          static_cast<DynamicallyAllocatedCallback*>(_events[i].data.ptr);
      (*cb_p)();
      delete cb_p;
    }

    // EINTR is sometimes returned epoll_wait under normal conditions.
    if (returned_events_count < 0 && errno != EINTR) {
      PLOG(ERROR) << "epoll_wait returned an error.";
    }

    // Check for destruction and end loop if so, unless there is still possibly
    // sockets still waiting.
    std::shared_lock<std::shared_timed_mutex> lock(_destructing_lock);
    if (_destructing &&
        returned_events_count != static_cast<int>(_events.size())) {
      return;
    }
  }
}

void LinuxCommunicationHandler::SocketReady(int soc, bool connected) {
  LOG(INFO) << "Handling a socket! socket=" << soc;
  CHECK(!connected);
  // connection established
  // check pending error if ever
  int err = 0;
  socklen_t len = sizeof(int);
  int soc_opt = getsockopt(soc, SOL_SOCKET, SO_ERROR, &err, &len);
  if (soc_opt == 0) {
    if (err == 0) {
      LOG(INFO) << "Connection established";
      // disable write event: to reenable when
      // data are to be sent on the socket
      epoll_event event;
      event.events = EPOLLIN;
      event.data.ptr = new DynamicallyAllocatedCallback(
          std::bind(&LinuxCommunicationHandler::Read, this, soc));
      int ret = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, soc, &event);
      PLOG_IF(ERROR, ret != 0)
          << "Failed to add socket to epoll set: socket=" << soc;

      char buf[] =
          "GET / HTTP/1.1\n"
          "Host: www.google.com\n"
          "User-Agent: curl/7.54.0\n"
          "Accept: */*\n"
          "\n";
      ret = send(soc, buf, std::strlen(buf), /*flags=*/0);
      PLOG_IF(ERROR, ret != static_cast<int>(std::strlen(buf)))
          << "Failed to send: socket=" << soc << "\n"
          << buf;
    } else {
      PLOG(ERROR) << "Error in socket connect.";
      // epoll_event event;
      // event.events = 0;
      // event.data.fd = fd; // user data
      // epoll_ctl (epollfd, EPOLL_CTL_DEL,
      //             fd, &event);
      // close (fd);
    }
  } else {
    PLOG(ERROR) << "Failed to get sock options: socket=" << soc;
  }
}

void LinuxCommunicationHandler::Send(int soc) { LOG(INFO) << "Called Send!"; }

void LinuxCommunicationHandler::Read(int soc) {
  char buf[15];
  read(soc, buf, 15);
  LOG(INFO) << buf;
  epoll_event event;
  event.events = 0;
  epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, soc, &event);
  close(soc);
}

}  // namespace dans