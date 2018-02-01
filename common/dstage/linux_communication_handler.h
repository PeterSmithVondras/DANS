#ifndef DANS02_LINUX_COMMUNICATION_HANDLER_H
#define DANS02_LINUX_COMMUNICATION_HANDLER_H

#include <sys/epoll.h>

#include <functional>
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
  void Connect(const std::string& ip, const std::string& port,
               CallBack2 done) override;
  void Monitor(int soc, ReadyFor ready_for, CallBack2 done) override;
  void Close(int soc) override;

 private:
  using DynamicallyAllocatedCallback = std::function<void(uint32_t)>;
  void MonitorAllSockets();
  void MonitorSocketReady(int soc, CallBack2 done, uint32_t events);
  void ConnectionReady(int soc, CallBack2 done, uint32_t events);
  void Read(int soc, uint32_t events);

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
