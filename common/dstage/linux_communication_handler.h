#ifndef DANS02_LINUX_COMMUNICATION_HANDLER_H
#define DANS02_LINUX_COMMUNICATION_HANDLER_H

#include <sys/epoll.h>

#include <functional>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "common/dstage/communication_handler_interface.h"
#include "common/dstage/synchronization.h"
#include "common/util/callback.h"

namespace dans {

class LinuxCommunicationHandler : public CommunicationHandlerInterface {
 public:
  LinuxCommunicationHandler();
  ~LinuxCommunicationHandler() override;

  int CreateSocket() override;
  void Serve(unsigned short port, CallBack1 done) override;
  Deleter Connect(const std::string& ip, const std::string& port,
                  CallBack2 done) override;
  Deleter Monitor(int soc, ReadyFor ready_for, CallBack2 done) override;
  Deleter MonitorNew(int soc, ReadyFor ready_for, CallBack2 done) override;
  void Close(int soc) override;

  static void PrintEpollEvents(uint32_t events);

 private:
  // using DynamicallyAllocatedCallback = std::function<void(uint32_t)>;
  using DynamicallyAllocatedCallback = util::callback::Callback<uint32_t>;
  void MonitorAllSockets();
  Deleter MonitorFor(int option, int soc, uint32_t events, CallBack2 done);
  void MonitorSocketReady(int soc, CallBack2 done, uint32_t events);
  void ServeSocketReady(int soc, CallBack1 done, uint32_t events);
  void AddServerSocket(int option, int soc, CallBack1 done);
  int CheckForSocketErrors(int soc);

  int _epoll_fd;
  std::vector<struct epoll_event> _events;
  std::thread _socket_handler;
  std::vector<std::unique_ptr<Connection>> _servers;
  // Guards the destruction state.
  std::shared_timed_mutex _destructing_lock;
  // Signals the destruction phase to all threads.
  bool _destructing;
};
}  // namespace dans

#endif  // DANS02_LINUX_COMMUNICATION_HANDLER_H
