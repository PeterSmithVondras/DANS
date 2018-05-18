#ifndef DANS02_PROXY_DSTAGE_H
#define DANS02_PROXY_DSTAGE_H

#include <csignal>
#include <functional>
#include <mutex>

#include "common/dstage/communication_handler_interface.h"
#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/executor.h"
#include "common/dstage/forwarding_dispatcher.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"
#include "common/dstage/synchronization.h"
#include "common/dstage/throttler.h"
#include "common/util/callback.h"

namespace dans {

struct HalfPipe {
  HalfPipe() {}
  HalfPipe(int soc) : connection(std::make_unique<Connection>(soc)) {}
  std::unique_ptr<Connection> connection;
};

class TcpPipe {
 public:
  enum PipeResults { READ_FAIL, SEND_FAIL, TRY_LATER };

  TcpPipe() = delete;
  TcpPipe(int soc)
      : in(std::make_unique<HalfPipe>(soc)),
        first_cb(nullptr),
        second_cb(nullptr) {}
  TcpPipe(std::unique_ptr<HalfPipe> in)
      : in(std::move(in)), first_cb(nullptr), second_cb(nullptr) {}
  TcpPipe(std::unique_ptr<HalfPipe> in, std::unique_ptr<HalfPipe> out)
      : in(std::move(in)),
        out(std::move(out)),
        first_cb(nullptr),
        second_cb(nullptr) {}

  std::string Describe();
  std::string Which(int soc);
  int OtherSocket(int soc);
  void ShutdownOther(int soc);
  // Pipes from connections related to soc to the other.
  // Returns:
  //    Success: total bytes piped or zero if soc was closed by peer.
  //    Failure: The negative of the socket which had a failure and sets errno.
  int Pipe(int soc);

  std::unique_ptr<HalfPipe> in;
  std::unique_ptr<HalfPipe> out;
  DynamicallyAllocatedCallback* first_cb;
  DynamicallyAllocatedCallback* second_cb;
};

class ProxyScheduler : public Scheduler<std::unique_ptr<TcpPipe>> {
 public:
  ProxyScheduler(std::vector<unsigned> threads_per_prio,
                 bool set_thread_priority,
                 CommunicationHandlerInterface* comm_interface);
  ~ProxyScheduler();

  void LinkMultiQ(BaseMultiQueue<std::unique_ptr<TcpPipe>>* multi_q_p) override;

  Throttler<std::unique_ptr<dans::TcpPipe>>* GetThrottler() {
    return _throttler.get();
  }

 protected:
  void StartScheduling(Priority prio) override;

 private:
  void ConnectCallback(SharedThrottleJobPtr<std::unique_ptr<TcpPipe>> job,
                       int soc,
                       CommunicationHandlerInterface::ReadyFor ready_for);
  void ConnectCallbackWrapper(
      SharedThrottleJobPtr<std::unique_ptr<TcpPipe>> job, int soc,
      CommunicationHandlerInterface::ReadyFor ready_for);

  void MonitorCallback(SharedThrottleJobPtr<std::unique_ptr<TcpPipe>> job,
                       int soc,
                       CommunicationHandlerInterface::ReadyFor ready_for);
  void MonitorCallbackWrapper(
      SharedThrottleJobPtr<std::unique_ptr<TcpPipe>> job, int soc,
      CommunicationHandlerInterface::ReadyFor ready_for);

  void CliClosedCallback(SharedThrottleJobPtr<std::unique_ptr<TcpPipe>> job,
                         int soc,
                         CommunicationHandlerInterface::ReadyFor ready_for);
  void CliClosedCallbackWrapper(
      SharedThrottleJobPtr<std::unique_ptr<TcpPipe>> job, int soc,
      CommunicationHandlerInterface::ReadyFor ready_for);

  CommunicationHandlerInterface* _comm_interface;
  bool _destructing;
  std::shared_timed_mutex _destructing_lock;

  dans::Executor _worker_exec;
  const std::string _primary_prio_port_out;
  const std::string _secondary_prio_port_out;
  const std::string _server_ip;
  std::unique_ptr<Throttler<std::unique_ptr<TcpPipe>>> _throttler;
};

class DStageProxy
    : public DStage<std::unique_ptr<TcpPipe>, std::unique_ptr<TcpPipe>> {
 public:
  DStageProxy(std::vector<unsigned> threads_per_prio, bool set_thread_priority,
              CommunicationHandlerInterface* comm_interface)
      : DStage<std::unique_ptr<TcpPipe>, std::unique_ptr<TcpPipe>>(
            threads_per_prio.size() - 1,
            std::make_unique<MultiQueue<std::unique_ptr<TcpPipe>>>(
                threads_per_prio.size() - 1),
            std::make_unique<ForwardingDispatcher<std::unique_ptr<TcpPipe>>>(
                threads_per_prio.size() - 1),
            std::make_unique<ProxyScheduler>(
                threads_per_prio, set_thread_priority, comm_interface)) {
    // Ignore SIGPIPE as this will happen often and is expected.
    std::signal(SIGPIPE, SIG_IGN);
  }

  Throttler<std::unique_ptr<dans::TcpPipe>>* GetThrottler() {
    return dynamic_cast<ProxyScheduler*>(_scheduler.get())->GetThrottler();
  }
};

}  // namespace dans

#endif  // DANS02_PROXY_DSTAGE_H
