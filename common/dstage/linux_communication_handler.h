#ifndef DANS02_LINUX_COMMUNICATION_HANDLER_H
#define DANS02_LINUX_COMMUNICATION_HANDLER_H

#include <sys/epoll.h>

#include <shared_mutex>
#include <thread>
#include <vector>

#include "common/dstage/communication_handler_interface.h"

namespace dans {

class LinuxCommunicationHandler : public CommunicationHandlerInterface {
 public:
  LinuxCommunicationHandler();
  ~LinuxCommunicationHandler() override;

  int CreateSocket() override;
  void Connect() override;

 private:
  using DynamicallyAllocatedCallback = std::function<void()>;
  void MonitorSockets();
  void SocketReady(int soc, bool connected);
  static void Send(int soc);
  void Read(int soc);

  int _epoll_fd;
  std::vector<struct epoll_event> _events;
  std::thread _socket_handler;
  // Guards the destruction state.
  std::shared_timed_mutex _destructing_lock;
  // Signals the destruction phase to all threads.
  bool _destructing;
};
}  // namespace dans

#endif  // DANS02_LINUX_COMMUNICATION_HANDLER_H