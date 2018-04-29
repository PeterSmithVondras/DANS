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
  LinuxCommunicationHandler(unsigned number_of_workers);
  ~LinuxCommunicationHandler() override;

  int CreateSocket() override;
  void Serve(unsigned short port, CallBack1 done) override;
  DynamicallyAllocatedCallback* Connect(const std::string& ip,
                                        const std::string& port,
                                        CallBack2 done) override;
  DynamicallyAllocatedCallback* Monitor(int soc, ReadyFor ready_for,
                                        CallBack2 done) override;
  DynamicallyAllocatedCallback* MonitorNew(int soc, ReadyFor ready_for,
                                           CallBack2 done) override;

  UniqCbp Connect2(const std::string& ip, const std::string& port,
                   CallBack2 done, CallbackDeleteOption delete_option) override;
  UniqCbp Monitor2(int soc, ReadyFor ready_for, CallBack2 done,
                   CallbackDeleteOption delete_option) override;
  UniqCbp MonitorNew2(int soc, ReadyFor ready_for, CallBack2 done,
                      CallbackDeleteOption delete_option) override;

  void Close(int soc) override;

  static void PrintEpollEvents(uint32_t events);

 private:
  void MonitorAllSockets();
  UniqCbp MonitorFor(int option, int soc, uint32_t events, CallBack2 done,
                     CallbackDeleteOption delete_option);
  void MonitorSocketReady(int soc, CallBack2 done, uint32_t events);
  void ServeSocketReady(int soc, CallBack1 done, uint32_t events);
  void AddServerSocket(int option, int soc, CallBack1 done);
  int CheckForSocketErrors(int soc);

  int _epoll_fd;
  std::vector<struct epoll_event> _events;
  // std::thread _socket_handler;
  std::vector<std::thread> _socket_handlers;
  std::vector<std::unique_ptr<Connection>> _servers;
  // Guards the destruction state.
  std::shared_timed_mutex _destructing_lock;
  // Signals the destruction phase to all threads.
  bool _destructing;
};
}  // namespace dans

#endif  // DANS02_LINUX_COMMUNICATION_HANDLER_H
