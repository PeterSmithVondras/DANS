#ifndef DANS02_PROXY_DSTAGE_H
#define DANS02_PROXY_DSTAGE_H

#include <functional>
#include <mutex>

#include "common/dstage/communication_handler_interface.h"
#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/forwarding_dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"
#include "common/dstage/synchronization.h"

namespace dans {

struct ClientConnection {
  std::unique_ptr<Connection> incoming_connection;
  std::function<void()> deleter;
};

struct HalfPipe {
  HalfPipe() : index(-1) {}
  std::unique_ptr<Connection> connection;
  std::unique_ptr<std::vector<char>> to_send;
  int index;
  std::function<void()> deleter;
};

class TcpPipe {
 public:
  TcpPipe() = delete;
  TcpPipe(std::unique_ptr<Connection> connection,
          std::function<void()> in_deleter);

 private:
  std::mutex lock;
  // std::shared_ptr<PurgeState> purge_state;
  HalfPipe in;
  HalfPipe out;
};

class ProxyScheduler : public Scheduler<ClientConnection> {
 public:
  ProxyScheduler(std::vector<unsigned> threads_per_prio,
                 bool set_thread_priority,
                 CommunicationHandlerInterface* comm_interface);
  ~ProxyScheduler();

 protected:
  // void StartScheduling(Priority prio) override;

 private:
  CommunicationHandlerInterface* _comm_interface;
  // BaseDStage<ResponseData>* _response_dstage;
  bool _destructing;
  std::shared_timed_mutex _destructing_lock;
};

class DStageProxy : public DStage<ClientConnection, ClientConnection> {
 public:
  DStageProxy(std::vector<unsigned> threads_per_prio, bool set_thread_priority,
              CommunicationHandlerInterface* comm_interface)
      : DStage<ClientConnection, ClientConnection>(
            threads_per_prio.size() - 1,
            std::make_unique<MultiQueue<ClientConnection>>(
                threads_per_prio.size() - 1),
            std::make_unique<ForwardingDispatcher<ClientConnection>>(
                threads_per_prio.size() - 1),
            std::make_unique<ProxyScheduler>(
                threads_per_prio, set_thread_priority, comm_interface)) {}
};

}  // namespace dans

#endif  // DANS02_PROXY_DSTAGE_H
