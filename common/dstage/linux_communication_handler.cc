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
using util::callback::CallbackDeleteOption;
// Currently have no flags to pass to epoll_create.
int kFlags = 0;
int kMaxEvents = 64;
int kTimeoutInMilliseconds = 500;
int kMaxConnectionsWaiting = 100;
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
  for (const auto& server : _servers) {
    server->Shutdown();
  }

  // Let the server sockets free memory.
  sleep(1);

  {
    std::unique_lock<std::shared_timed_mutex> lock(_destructing_lock);
    _destructing = true;
  }
  _socket_handler.join();

  PLOG_IF(ERROR, close(_epoll_fd) != 0)
      << "Failure to close epoll file descriptor";
}

void LinuxCommunicationHandler::PrintEpollEvents(uint32_t events) {
  VLOG(3) << "Events: EPOLLIN=" << (EPOLLIN & events)
          << " EPOLLOUT=" << (EPOLLOUT & events)
          << " EPOLLRDHUP=" << (EPOLLRDHUP & events)
          << " EPOLLPRI=" << (EPOLLPRI & events)
          << " EPOLLERR=" << (EPOLLERR & events)
          << " EPOLLHUP=" << (EPOLLHUP & events)
          << " EPOLLET=" << (EPOLLET & events)
          << " EPOLLONESHOT=" << (EPOLLONESHOT & events)
          << " EPOLLWAKEUP=" << (EPOLLWAKEUP & events);
  // << " EPOLLEXCLUSIVE=" << (EPOLLEXCLUSIVE & events);
}

int LinuxCommunicationHandler::CreateSocket() {
  VLOG(4) << __PRETTY_FUNCTION__;
  int soc = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  PLOG_IF(ERROR, soc == -1) << "Failed to create socket.";
  return soc;
}

void LinuxCommunicationHandler::AddServerSocket(int option, int soc,
                                                CallBack1 done) {
  struct epoll_event event;
  // Need to wait on socket for ability to accept.
  event.events = EPOLLERR | EPOLLRDHUP | EPOLLHUP | EPOLLONESHOT | EPOLLIN;

  // Setting user data in the event structure to be a callback with relevant
  // data.
  event.data.ptr = new DynamicallyAllocatedCallback(
      std::bind(&LinuxCommunicationHandler::ServeSocketReady, this, soc, done,
                std::placeholders::_1),
      CallbackDeleteOption::DELETE_AFTER_CALLING);
  int ret = epoll_ctl(_epoll_fd, option, soc, &event);
  PLOG_IF(ERROR, ret != 0)
      << "SERVING ERROR: Failed to add socket to epoll set socket=" << soc;
}

void LinuxCommunicationHandler::Serve(unsigned short port, CallBack1 done) {
  VLOG(4) << __PRETTY_FUNCTION__;
  int soc = CreateSocket();
  _servers.push_back(std::make_unique<Connection>(soc));
  VLOG(1) << "Created socket for listening: socket=" << soc << " port=" << port;

  int yes = 1;
  PLOG_IF(ERROR,
          setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
      << "Failed to set listen socket options.";

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  PLOG_IF(ERROR, bind(soc, reinterpret_cast<struct sockaddr*>(&addr),
                      sizeof(sockaddr_in)) == -1)
      << "Failed to bind listen socket.";

  PLOG_IF(ERROR, listen(soc, kMaxConnectionsWaiting) == -1)
      << "Failed to listen to port=" << port << " on socket=" << soc;

  AddServerSocket(EPOLL_CTL_ADD, soc, done);
}

void LinuxCommunicationHandler::ServeSocketReady(int soc, CallBack1 done,
                                                 uint32_t events) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  if (!(events & EPOLLIN)) {
    VLOG(2) << "Server socket=" << soc << " closing.";
    return;
  }

  VLOG(1) << "Server socket=" << soc << " handling new connections.";
  // Must re-add server socket to epoll
  AddServerSocket(EPOLL_CTL_MOD, soc, done);

  sockaddr_in client_addr;
  socklen_t len = sizeof(sockaddr_in);
  int client_soc;
  // Must get all pending connections.
  while (true) {
    client_soc = accept4(soc, reinterpret_cast<sockaddr*>(&client_addr), &len,
                         SOCK_NONBLOCK);
    if (client_soc == -1) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        // connection already closed or interrupted
        break;
      } else {
        PLOG(ERROR) << "Failed to accept client connection.";
      }
    } else {
      // New connection is successful.
      VLOG(2) << "Server socket=" << soc
              << " accepted new connection socket=" << client_soc;
      done(client_soc);
    }
  }
}

std::function<void()> LinuxCommunicationHandler::Connect(
    const std::string& ip, const std::string& port, CallBack2 done) {
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
    done(soc, ReadyFor{/*in=*/false, /*out=*/false});
    return []() {};
  } else if (ret == 0) {
    done(soc, {/*in=*/false, /*out=*/false});
    return []() {};
  }

  struct epoll_event event;
  // Need to wait on socket for ability to connect.
  event.events =
      EPOLLERR | EPOLLRDHUP | EPOLLHUP | EPOLLIN | EPOLLOUT | EPOLLONESHOT;

  // Setting user data in the event structure to be a callback with relevant
  // data.
  auto cb_p = new DynamicallyAllocatedCallback(
      std::bind(&LinuxCommunicationHandler::MonitorSocketReady, this, soc, done,
                std::placeholders::_1),
      CallbackDeleteOption::DELETE_AFTER_CALLING);
  event.data.ptr = cb_p;
  ret = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, soc, &event);
  PLOG_IF(WARNING, ret != 0)
      << "Failed to add socket to epoll set during connect: socket=" << soc;

  return Deleter([cb_p, soc]() {
    delete cb_p;
    VLOG(2) << "Deleted LinuxCommunicationHandler callback successfully for "
               "socket="
            << soc;
  });
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
      VLOG(2) << "MonitorAllSockets reactor handling a socket.";
      auto cb_p =
          static_cast<DynamicallyAllocatedCallback*>(_events[i].data.ptr);
      (*cb_p)(_events[i].events);
      // delete cb_p;
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

std::function<void()> LinuxCommunicationHandler::Monitor(int soc,
                                                         ReadyFor ready_for,
                                                         CallBack2 done) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  uint32_t events = 0;
  if (ready_for.in) events |= EPOLLIN;
  if (ready_for.out) events |= EPOLLOUT;
  return MonitorFor(EPOLL_CTL_MOD, soc, events, done);
}

std::function<void()> LinuxCommunicationHandler::MonitorNew(int soc,
                                                            ReadyFor ready_for,
                                                            CallBack2 done) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  uint32_t events = 0;
  if (ready_for.in) events |= EPOLLIN;
  if (ready_for.out) events |= EPOLLOUT;
  return MonitorFor(EPOLL_CTL_ADD, soc, events, done);
}

std::function<void()> LinuxCommunicationHandler::MonitorFor(int option, int soc,
                                                            uint32_t events,
                                                            CallBack2 done) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  CHECK_GE(soc, 0);

  epoll_event event;
  event.events = EPOLLERR | EPOLLRDHUP | EPOLLHUP | EPOLLONESHOT | events;

  auto cb_p = new DynamicallyAllocatedCallback(
      std::bind(&LinuxCommunicationHandler::MonitorSocketReady, this, soc, done,
                std::placeholders::_1),
      CallbackDeleteOption::DELETE_AFTER_CALLING);
  event.data.ptr = cb_p;
  int ret = epoll_ctl(_epoll_fd, option, soc, &event);
  PLOG_IF(ERROR, ret != 0) << "Failed to "
                           << (option == EPOLL_CTL_ADD ? "add" : "reintroduce")
                           << " socket to epoll set during MonitorFor: socket="
                           << soc;

  return Deleter([cb_p, soc]() {
    delete cb_p;
    VLOG(2) << "Deleted LinuxCommunicationHandler callback successfully for "
               "socket="
            << soc;
  });
}

int LinuxCommunicationHandler::CheckForSocketErrors(int soc) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  // check pending error if ever
  int err = 0;
  socklen_t len = sizeof(int);
  int soc_opt = getsockopt(soc, SOL_SOCKET, SO_ERROR, &err, &len);
  if (soc_opt != 0) {
    PLOG(WARNING) << "Failed to get sock options: socket=" << soc;
  } else {
    if (err != 0) {
      errno = err;
    }
  }
  return err;
}

void LinuxCommunicationHandler::MonitorSocketReady(int soc, CallBack2 done,
                                                   uint32_t events) {
  VLOG(4) << __PRETTY_FUNCTION__ << " soc=" << soc;
  VLOG(1) << "Monitor handling a socket=" << soc;
  // connection established
  // check pending error if ever
  int err = CheckForSocketErrors(soc);
  if (err != 0) {
    errno = err;
    done(soc, {/*in=*/false, /*out=*/false, events, err});
  } else {
    // Custom for this function.
    ReadyFor ready{/*in=*/false, /*out=*/false, events, err};
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
