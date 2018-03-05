#ifndef DANS02_CLIENT_RESPONSE_HANDLER_H
#define DANS02_CLIENT_RESPONSE_HANDLER_H

#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>

#include "common/dstage/client_request_handler.h"
#include "common/dstage/communication_handler_interface.h"
#include "common/dstage/dispatcher.h"
#include "common/dstage/dstage.h"
#include "common/dstage/job.h"
#include "common/dstage/job_types.h"
#include "common/dstage/multiqueue.h"
#include "common/dstage/scheduler.h"

namespace dans {

class ResponseDispatcher : public Dispatcher<ResponseData, ResponseData> {
 public:
  ResponseDispatcher(Priority max_priority);

 protected:
  void DuplicateAndEnqueue(UniqJobPtr<ResponseData> job_in, Priority max_prio,
                           unsigned duplication) override;
};

class ResponseScheduler : public Scheduler<ResponseData> {
 public:
  ResponseScheduler(std::vector<unsigned> threads_per_prio,
                    bool set_thread_priority,
                    CommunicationHandlerInterface* comm_interface,
                    BaseDStage<ResponseData>* response_handler);
  ~ResponseScheduler();

  void RegisterOriginDStage(BaseDStage<ConnectData>* origin_dstage);

 protected:
  void StartScheduling(Priority prio) override;

 private:
  CommunicationHandlerInterface* _comm_interface;
  BaseDStage<ResponseData>* _response_handler;
  bool _destructing;
  std::shared_timed_mutex _destructing_lock;
  BaseDStage<ConnectData>* _origin_dstage;

  void ResponseCallback(SharedJobPtr<ResponseData> old_job, int soc,
                        CommunicationHandlerInterface::ReadyFor ready_for);
};

class ResponseDStage : public DStage<ResponseData, ResponseData> {
 public:
  ResponseDStage(std::vector<unsigned> threads_per_prio,
                 bool set_thread_priority,
                 CommunicationHandlerInterface* comm_interface)
      : DStage<ResponseData, ResponseData>(
            threads_per_prio.size() - 1,
            std::make_unique<MultiQueue<ResponseData>>(threads_per_prio.size() -
                                                       1),
            std::make_unique<ResponseDispatcher>(threads_per_prio.size() - 1),
            std::make_unique<ResponseScheduler>(
                threads_per_prio, set_thread_priority, comm_interface, this)) {}

  void RegisterOriginDStage(BaseDStage<ConnectData>* origin_dstage) {
    static_cast<ResponseScheduler*>(_scheduler.get())
        ->RegisterOriginDStage(origin_dstage);
  }
};

}  // namespace dans

#endif  // DANS02_CLIENT_RESPONSE_HANDLER_H