#ifndef DANS02_CLIENT_REQUEST_HANDLER_H
#define DANS02_CLIENT_REQUEST_HANDLER_H

#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>

#include "common/dstage/communication_handler_interface.h"
#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/job.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

namespace dans {

struct ConnectData {
  std::vector<std::string> ip_addresses;
  std::vector<std::string> ports;
  std::shared_ptr<std::function<void(int)>> done;
};

struct ConnectDataInternal {
  std::string ip;
  std::string port;
  std::shared_ptr<std::function<void(int)>> done;
};

class ConnectDispatcher : public Dispatcher<ConnectData, ConnectDataInternal> {
 public:
  ConnectDispatcher(Priority max_priority);

 protected:
  void DuplicateAndEnqueue(UniqConstJobPtr<ConnectData> job_in,
                           Priority max_prio, unsigned duplication) override;
};

class ConnectScheduler : public Scheduler<ConnectDataInternal> {
 public:
  ConnectScheduler(std::vector<unsigned> threads_per_prio,
                   bool set_thread_priority);
  ~ConnectScheduler();

 protected:
  void StartScheduling(Priority prio) override;

 private:
  bool _destructing;
  std::shared_timed_mutex _destructing_lock;
};

class ConnectDStage : public DStage<ConnectData, ConnectDataInternal> {
 public:
  ConnectDStage(std::vector<unsigned> threads_per_prio,
                bool set_thread_priority,
                CommunicationHandlerInterface* comm_interface)
      : DStage<ConnectData, ConnectDataInternal>(
            threads_per_prio.size() - 1,
            std::make_unique<MultiQueue<ConnectDataInternal>>(
                threads_per_prio.size() - 1),
            std::make_unique<ConnectDispatcher>(threads_per_prio.size() - 1),
            std::make_unique<ConnectScheduler>(threads_per_prio,
                                               set_thread_priority)) {}
};

}  // namespace dans

#endif  // DANS02_CLIENT_REQUEST_HANDLER_H