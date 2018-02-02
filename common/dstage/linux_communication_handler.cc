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
      std::thread(&LinuxCommunicationHandler::MonitorAllSockets, this);
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

void LinuxCommunicationHandler::Connect(const std::string& ip,
                                        const std::string& port,
                                        CallBack2 done) {
  VLOG(4) << __PRETTY_FUNCTION__;
  int soc = CreateSocket();
  VLOG(1) << "Created socket for connect: socket=" << soc;

  const unsigned short port_num = atoi(port.c_str());
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_num);
  inet_aton(ip.c_str(), &addr.sin_addr);

  int ret =
      connect(soc, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in));
  if ((ret != 0) && (errno != EINPROGRESS)) {
    PLOG(WARNING) << "Failed to connect socket with initial call to connect().";
    done(-errno, ReadyFor{/*in=*/false, /*out=*/false});
    return;
  } else if (ret == 0) {
    done(soc, {/*in=*/false, /*out=*/false});
    return;
  }

  struct epoll_event event;
  // Need to wait on socket for ability to connect.
  event.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;

  // Setting user data in the event structure to be a callback with relevant
  // data.
  event.data.ptr = new DynamicallyAllocatedCallback(
      std::bind(&LinuxCommunicationHandler::ConnectionReady, this, soc, done,
                std::placeholders::_1));
  ret = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, soc, &event);
  PLOG_IF(WARNING, ret != 0)
      << "Failed to add socket to epoll set: socket=" << soc;
}

void LinuxCommunicationHandler::MonitorAllSockets() {
  VLOG(4) << __PRETTY_FUNCTION__;
  while (true) {
    // Waiting for available socket.
    int returned_events_count = epoll_wait(
        _epoll_fd, _events.data(), _events.size(), kTimeoutInMilliseconds);
    VLOG(1) << "epoll_wait returned " << returned_events_count << " socket(s).";

    // Handle every returned socket.
    for (int i = 0; i < returned_events_count; i++) {
      VLOG(1) << "MonitorAllSockets reactor handling a socket.";
      auto cb_p =
          static_cast<DynamicallyAllocatedCallback*>(_events[i].data.ptr);
      (*cb_p)(_events[i].events);
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

void LinuxCommunicationHandler::ConnectionReady(int soc, CallBack2 done,
                                                uint32_t events) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  // connection established
  // check pending error if ever
  int err = CheckForSocketErrors(soc);
  if (err < 0) {
    done(err, {/*in=*/false, /*out=*/false});
  } else if (events & EPOLLOUT) {
    VLOG(1) << "TCP connection established for socket=" << soc;
    ReadyFor ready{/*in=*/false, /*out=*/true};
    if (events & EPOLLIN) {
      ready.in = true;
    }
    done(soc, ready);
  } else if (events & EPOLLIN) {
    done(soc, ReadyFor{/*in=*/true, /*out=*/false});
  } else {
    LOG(WARNING) << "Connection socket error unknown: socket=" << soc;
    done(-EBADFD, {/*in=*/false, /*out=*/false});
  }
}

void LinuxCommunicationHandler::Monitor(int soc, ReadyFor ready_for,
                                        CallBack2 done) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  CHECK_GE(soc, 0);
  if (!ready_for.in && !ready_for.out) {
    LOG(WARNING) << "Request to monitor nothing for socket=" << soc;
    done(soc, ready_for);
    return;
  }
  epoll_event event;
  event.events = EPOLLONESHOT;
  if (ready_for.in) event.events |= EPOLLIN;
  if (ready_for.out) event.events |= EPOLLOUT;

  event.data.ptr = new DynamicallyAllocatedCallback(
      std::bind(&LinuxCommunicationHandler::MonitorSocketReady, this, soc, done,
                std::placeholders::_1));
  int ret = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, soc, &event);
  PLOG_IF(ERROR, ret != 0) << "Failed to add socket to epoll set: socket="
                           << soc;
}

int LinuxCommunicationHandler::CheckForSocketErrors(int soc) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  // check pending error if ever
  int err = 0;
  socklen_t len = sizeof(int);
  int soc_opt = getsockopt(soc, SOL_SOCKET, SO_ERROR, &err, &len);
  if (soc_opt != 0) {
    PLOG(WARNING) << "Failed to get sock options: socket=" << soc;
    close(soc);
    PLOG_IF(WARNING, close(soc) != 0) << "Failed to close socket=" << soc;
    soc = -errno;
  } else {
    if (err != 0) {
      PLOG(WARNING) << "Error in socket connect.";
      close(soc);
      PLOG_IF(WARNING, close(soc) != 0) << "Failed to close socket=" << soc;
      soc = -errno;
    }
  }

  return soc;
}

void LinuxCommunicationHandler::MonitorSocketReady(int soc, CallBack2 done,
                                                   uint32_t events) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  VLOG(1) << "Monitor handling a socket=" << soc;
  // connection established
  // check pending error if ever
  int err = CheckForSocketErrors(soc);
  if (err < 0) {
    done(err, {/*in=*/false, /*out=*/false});
  } else {
    // Custom for this function.
    ReadyFor ready{/*in=*/false, /*out=*/false};
    if (events & EPOLLIN) ready.in = true;
    if (events & EPOLLOUT) ready.out = true;
    done(soc, ready);
  }
}

void LinuxCommunicationHandler::Close(int soc) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  if (soc < 0) {
    LOG(WARNING) << "Cannot close negative socket=" << soc;
    return;
  }
  PLOG_IF(WARNING, close(soc) != 0) << "Failed to close socket=" << soc;
}

}  // namespace dans
